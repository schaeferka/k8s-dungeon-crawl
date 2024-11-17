/*
Copyright 2024.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package controller

import (
	"context"
	"fmt"
	kaschaeferv1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	appsv1 "k8s.io/api/apps/v1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/runtime"
	"k8s.io/client-go/util/retry"
	"k8s.io/utils/pointer"
	ctrl "sigs.k8s.io/controller-runtime"
	"sigs.k8s.io/controller-runtime/pkg/client"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	"sigs.k8s.io/controller-runtime/pkg/log"
)

// MonsterReconciler reconciles a Monster object
type MonsterReconciler struct {
	client.Client
	Scheme *runtime.Scheme
}

// +kubebuilder:rbac:groups=kaschaefer.com,resources=monsters,verbs=get;list;watch;create;update;patch;delete
// +kubebuilder:rbac:groups=kaschaefer.com,resources=monsters/status,verbs=get;update;patch
// +kubebuilder:rbac:groups=kaschaefer.com,resources=monsters/finalizers,verbs=update

// Reconcile is part of the main kubernetes reconciliation loop which aims to
// move the current state of the cluster closer to the desired state.
// TODO(user): Modify the Reconcile function to compare the state specified by
// the Monster object against the actual cluster state, and then
// perform operations to make the cluster state reflect the state specified by
// the user.
//
// For more details, check Reconcile and its Result here:
// - https://pkg.go.dev/sigs.k8s.io/controller-runtime@v0.19.1/pkg/reconcile
func (r *MonsterReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
	_ = log.FromContext(ctx)

	// Fetch the Monster object
	var monster kaschaeferv1.Monster
	if err := r.Get(context.Background(), req.NamespacedName, &monster); err != nil {
		// Handle error fetching the resource
		return ctrl.Result{}, client.IgnoreNotFound(err)
	}

	// Check if the monster is being deleted
	if monster.ObjectMeta.DeletionTimestamp != nil {
		// If the monster is being deleted, handle the finalizer cleanup
		if containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
			// Cleanup the associated resources (e.g., deployments)
			err := r.handleFinalizerForMonster(&monster)
			if err != nil {
				return ctrl.Result{}, err
			}

			// Remove the finalizer and allow the resource to be deleted
			controllerutil.RemoveFinalizer(&monster, "kaschaefer.com/cleanup")
			err = r.Update(context.Background(), &monster)
			if err != nil {
				return ctrl.Result{}, err
			}
		}
		return ctrl.Result{}, nil // No need to do further processing if deleting
	}

	// Add the finalizer if not already present
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		controllerutil.AddFinalizer(&monster, "kaschaefer.com/cleanup")
		err := r.Update(context.Background(), &monster)
		if err != nil {
			return ctrl.Result{}, err
		}
	}

	// Create or update ConfigMap with Monster data
	cm := &corev1.ConfigMap{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("monster-%s", monster.Name),
			Namespace: "monsters",
		},
		Data: map[string]string{
			"index.html": fmt.Sprintf(`
				<!DOCTYPE html>
				<html lang="en">
				<head>
					<meta charset="UTF-8">
					<meta name="viewport" content="width=device-width, initial-scale=1.0">
					<title>Monster Info: %s</title>
					<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css">
				</head>
				<body class="bg-gray-100 text-gray-900 flex flex-col min-h-screen">
					<header class="bg-blue-900 text-white p-4">
						<div class="container mx-auto flex items-center justify-between">
							<h1 class="text-3xl font-bold">K8s Dungeon Crawl</h1>
						</div>
					</header>

					<main class="container mx-auto my-8 flex-grow">
						<div class="bg-white p-6 rounded-lg shadow-lg">
							<h2 class="text-2xl font-bold text-blue-900 mb-4">Monster: %s</h2>
							<ul class="list-disc pl-6 space-y-2">
								<li><strong>ID:</strong> %s</li>
								<li><strong>Type:</strong> %s</li>
								<li><strong>CurrentHP:</strong> %s</li>
								<li><strong>MaxHP:</strong> %s</li>
								<li><strong>Depth:</strong> %s</li>
							</ul>
						</div>
					</main>

					<footer class="bg-blue-900 text-white p-4 text-center">
						<p>&copy; 2024 K8s Dungeon Crawl</p>
					</footer>
				</body>
				</html>
			`, monster.Name, monster.Name, fmt.Sprintf("%d", monster.Spec.ID), monster.Spec.Type, fmt.Sprintf("%d", monster.Spec.CurrentHP), fmt.Sprintf("%d", monster.Spec.MaxHP), fmt.Sprintf("%d", monster.Spec.Depth)),
		},
	}

	// Create or Update the ConfigMap
	err := retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingCM corev1.ConfigMap
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("monster-%s", monster.Name),
		}, &existingCM)

		if err != nil && client.IgnoreNotFound(err) != nil {
			return err
		}

		if err == nil {
			existingCM.Data = cm.Data
			return r.Update(context.Background(), &existingCM)
		}

		return r.Create(context.Background(), cm)
	})

	if err != nil {
		return ctrl.Result{}, err
	}

	// Create or Update the Nginx Deployment
	deployment := &appsv1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
		},
		Spec: appsv1.DeploymentSpec{
			Replicas: pointer.Int32Ptr(1),
			Selector: &metav1.LabelSelector{
				MatchLabels: map[string]string{
					"app": fmt.Sprintf("nginx-%s", monster.Name),
				},
			},
			Template: corev1.PodTemplateSpec{
				ObjectMeta: metav1.ObjectMeta{
					Labels: map[string]string{
						"app": fmt.Sprintf("nginx-%s", monster.Name),
					},
				},
				Spec: corev1.PodSpec{
					Containers: []corev1.Container{
						{
							Name:  "nginx",
							Image: "nginx:latest",
							Ports: []corev1.ContainerPort{
								{
									ContainerPort: 80,
								},
							},
							VolumeMounts: []corev1.VolumeMount{
								{
									Name:      "monster-data",
									MountPath: "/usr/share/nginx/html/index.html",
									SubPath:   "index.html",
								},
							},
						},
					},
					Volumes: []corev1.Volume{
						{
							Name: "monster-data",
							VolumeSource: corev1.VolumeSource{
								ConfigMap: &corev1.ConfigMapVolumeSource{
									LocalObjectReference: corev1.LocalObjectReference{
										Name: fmt.Sprintf("monster-%s", monster.Name),
									},
								},
							},
						},
					},
				},
			},
		},
	}

	err = retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingDeployment appsv1.Deployment
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
		}, &existingDeployment)

		if err != nil && client.IgnoreNotFound(err) != nil {
			return err
		}

		if err == nil {
			existingDeployment.Spec = deployment.Spec
			return r.Update(context.Background(), &existingDeployment)
		}

		return r.Create(context.Background(), deployment)
	})

	if err != nil {
		return ctrl.Result{}, err
	}

	// Return success
	return ctrl.Result{}, nil
}

// containsString checks if a string is present in a slice of strings
func containsString(slice []string, s string) bool {
	for _, item := range slice {
		if item == s {
			return true
		}
	}
	return false
}

// Finalizer function to clean up resources when the monster is deleted
func (r *MonsterReconciler) handleFinalizerForMonster(monster *kaschaeferv1.Monster) error {
	// Ensure finalizer is present
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		monster.Finalizers = append(monster.Finalizers, "kaschaefer.com/cleanup")
		err := r.Update(context.Background(), monster)
		if err != nil {
			return err
		}
	}

	// Cleanup associated resources (e.g., delete deployment)
	err := r.deleteDeployment(monster.Name, "monsters")
	if err != nil {
		return err
	}

	// Remove finalizer and allow deletion of the object
	monster.Finalizers = removeString(monster.Finalizers, "kaschaefer.com/cleanup")
	err = r.Update(context.Background(), monster)
	if err != nil {
		return err
	}

	return nil
}

// removeString removes a string from a slice of strings
func removeString(slice []string, s string) []string {
	for i, item := range slice {
		if item == s {
			return append(slice[:i], slice[i+1:]...)
		}
	}
	return slice
}

func (r *MonsterReconciler) deleteDeployment(name, namespace string) error {
	// Prepend 'nginx-' to the monster name
	name = fmt.Sprintf("nginx-%s", name)
	namespace = "monsters"

	// Delete the Nginx deployment associated with the monster
	deployment := &appsv1.Deployment{}
	err := r.Get(context.Background(), client.ObjectKey{Namespace: namespace, Name: name}, deployment)
	if err != nil {
		return err
	}

	// Delete the deployment
	err = r.Delete(context.Background(), deployment)
	if err != nil {
		return err
	}

	return nil
}

// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&kaschaeferv1.Monster{}).
		Owns(&corev1.ConfigMap{}). // Watch for ConfigMap changes as well
		Complete(r)
}
