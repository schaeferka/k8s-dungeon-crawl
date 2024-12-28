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
)

// int32Ptr returns a pointer to an int32 value
func int32Ptr(i int32) *int32 {
	return &i
}

// createOrUpdateDeployment ensures the Nginx Deployment is created or updated
func (r *MonsterReconciler) createOrUpdateDeployment(ctx context.Context, monster v1.Monster) error {
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
	cmHash := fmt.Sprintf("%x", hashData(cm.Data)) // Assuming `hashData` is a function to compute the hash

	// Create or Update the Nginx Deployment
	deployment := &appsv1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
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
