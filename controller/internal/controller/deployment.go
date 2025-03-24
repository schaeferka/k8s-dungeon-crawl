package controller

import (
	"context"
	"fmt"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	appsv1 "k8s.io/api/apps/v1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/util/intstr"
	"k8s.io/client-go/util/retry"
	"sigs.k8s.io/controller-runtime/pkg/client"
	log "sigs.k8s.io/controller-runtime/pkg/log"
	apierrors "k8s.io/apimachinery/pkg/api/errors"
)

// int32Ptr returns a pointer to an int32 value
func int32Ptr(i int32) *int32 {
	return &i
}

// createOrUpdateDeployment ensures the Nginx Deployment is created or updated
func (r *MonsterReconciler) createOrUpdateDeployment(ctx context.Context, monster v1.Monster) error {
	log.FromContext(ctx).Info("Deployment - Creating or Updating Deployment", "monster", monster.Name)

	// Skip updates if Monster is being deleted
	if monster.DeletionTimestamp != nil {
		log.FromContext(ctx).Info("Skipping Deployment update as Monster is being deleted", "name", monster.Name)
		return nil
	}

	// Fetch the existing ConfigMap to generate a hash for triggering rolling update
	var cm corev1.ConfigMap
	err := r.Get(ctx, client.ObjectKey{
		Namespace: "monsters",
		Name:      fmt.Sprintf("monster-%s", monster.Name),
	}, &cm)
	if err != nil {
		log.FromContext(ctx).Error(err, "unable to fetch ConfigMap for Monster")
		return err
	}

	// Generate a hash for the ConfigMap data to trigger the deployment update when the config changes
	cmHash := fmt.Sprintf("%x", hashData(cm.Data))

	// Create or Update the Nginx Deployment
	deployment := &appsv1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
			Labels: map[string]string{
				"owner": monster.Name,
			},
			//OwnerReferences: []metav1.OwnerReference{
			//	*metav1.NewControllerRef(&monster, v1.GroupVersion.WithKind("Monster")),
			//},
			Annotations: map[string]string{
				"configHash": cmHash, // Add a hash of the ConfigMap to trigger rolling update
			},
		},
		Spec: appsv1.DeploymentSpec{
			Replicas: int32Ptr(1), // Set to 1 replica
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
					Annotations: map[string]string{
						"configHash": cmHash, // Ensure the pod template also has this annotation
					},
				},
				Spec: corev1.PodSpec{
					Containers: []corev1.Container{
						{
							Name:  "nginx",
							Image: "nginx:latest", // Use the latest Nginx image
							Ports: []corev1.ContainerPort{
								{
									ContainerPort: 8000,
								},
							},
							VolumeMounts: []corev1.VolumeMount{
								{
									Name:      "monster-data",
									MountPath: "/usr/share/nginx/html/index.html", // Nginx serves the content from here
									SubPath:   "index.html",                       // Only mount the specific file, not the whole config map
								},
								{
									Name:      "nginx-config",
									MountPath: "/etc/nginx/nginx.conf", // Mount the nginx.conf
									SubPath:   "nginx.conf",            // Only mount the specific nginx config
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
						{
							Name: "nginx-config",
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
			// Add rolling update strategy to ensure pods are updated when ConfigMap changes
			Strategy: appsv1.DeploymentStrategy{
				Type: appsv1.RollingUpdateDeploymentStrategyType,
				RollingUpdate: &appsv1.RollingUpdateDeployment{
					MaxUnavailable: &intstr.IntOrString{IntVal: 1},
					MaxSurge:       &intstr.IntOrString{IntVal: 1},
				},
			},
		},
	}

	// Apply or update the deployment
	err = retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingDeployment appsv1.Deployment
		err := r.Get(ctx, client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
		}, &existingDeployment)

		if err != nil && client.IgnoreNotFound(err) != nil {
			log.FromContext(ctx).Error(err, "unable to fetch Deployment for Monster")
			return err
		}

		if err == nil {
			// If the deployment exists, update it
			existingDeployment.Spec = deployment.Spec
			// Trigger a rolling update of the deployment
			return r.Update(ctx, &existingDeployment)
		}

		// Create new deployment if not found
		return r.Create(ctx, deployment)
	})

	if err != nil {
		log.FromContext(ctx).Error(err, "unable to create or update Deployment for Monster")
	}

	return err
}

func (r *MonsterReconciler) deleteDeployment(ctx context.Context, name, namespace string) error {
	// Prepend 'nginx-' to the monster name
	name = fmt.Sprintf("nginx-%s", name)
	log.FromContext(ctx).Info("Deployment - Starting deleteDeployment for Monster", "name", name)

	// Fetch the deployment
	deployment := &appsv1.Deployment{}
	if err := r.Get(ctx, client.ObjectKey{Namespace: namespace, Name: name}, deployment); err != nil {
		if apierrors.IsNotFound(err) {
			log.FromContext(ctx).Info("Deployment - Already deleted", "name", name)
			return nil // Not an error if already deleted
		}
		log.FromContext(ctx).Error(err, "Deployment - Unable to fetch Deployment for Monster", "name", name)
		return err
	}

	// Delete the deployment
	log.FromContext(ctx).Info("Deployment - Deleting Deployment for Monster", "name", name)
	if err := r.Delete(ctx, deployment); err != nil {
		log.FromContext(ctx).Error(err, "Deployment - Unable to delete Deployment for Monster", "name", name)
		return err
	}

	return nil
}

