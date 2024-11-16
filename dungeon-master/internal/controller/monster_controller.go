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

	// Create or update ConfigMap with Monster data
	// Update the ConfigMap with monster-specific content
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
			// If error is not due to not found, return
			return err
		}

		if err == nil {
			// If the ConfigMap exists, update it
			existingCM.Data = cm.Data
			return r.Update(context.Background(), &existingCM)
		}

		// Create new ConfigMap if not found
		return r.Create(context.Background(), cm)
	})

	if err != nil {
		return ctrl.Result{}, err
	}

	// Create Nginx Deployment in the 'monsters' namespace when a Monster is created
	deployment := &appsv1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
		},
		Spec: appsv1.DeploymentSpec{
			Replicas: pointer.Int32Ptr(1), // Set to 1 replica
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
							Image: "nginx:latest", // Use the latest Nginx image
							Ports: []corev1.ContainerPort{
								{
									ContainerPort: 80, // Expose port 80 for HTTP
								},
							},
							VolumeMounts: []corev1.VolumeMount{
								{
									Name:      "monster-data",
									MountPath: "/usr/share/nginx/html/index.html", // Nginx serves the content from here
									SubPath:   "index.html",                       // Only mount the specific file, not the whole config map
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

	// Create or Update the Nginx Deployment
	err = retry.RetryOnConflict(retry.DefaultRetry, func() error {
		// Check if the deployment already exists
		var existingDeployment appsv1.Deployment
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
		}, &existingDeployment)

		if err != nil && client.IgnoreNotFound(err) != nil {
			// If error is not due to not found, return
			return err
		}

		if err == nil {
			// If the deployment exists, update it
			existingDeployment.Spec = deployment.Spec
			return r.Update(context.Background(), &existingDeployment)
		}

		// Create new Deployment if not found
		return r.Create(context.Background(), deployment)
	})

	if err != nil {
		return ctrl.Result{}, err
	}

	// Return success
	return ctrl.Result{}, nil
}

// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&kaschaeferv1.Monster{}).
		Owns(&corev1.ConfigMap{}). // Watch for ConfigMap changes as well
		Complete(r)
}
