package controller

import (
	"fmt"
	corev1 "k8s.io/api/core/v1"
	kaschaeferv1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	"k8s.io/client-go/util/retry"
	"context"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	intstr "k8s.io/apimachinery/pkg/util/intstr"
	client "sigs.k8s.io/controller-runtime/pkg/client"
	log "sigs.k8s.io/controller-runtime/pkg/log"
)

// createOrUpdateService creates or updates the service for the monster's Nginx deployment
func (r *MonsterReconciler) createOrUpdateService(ctx context.Context, monster kaschaeferv1.Monster) error {
	// Skip updates if Monster is being deleted
	if monster.DeletionTimestamp != nil {
		log.FromContext(ctx).Info("Skipping Service update as Monster is being deleted", "name", monster.Name)
		return nil
	}

	service := &corev1.Service{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
		},
		Spec: corev1.ServiceSpec{
			Selector: map[string]string{
				"app": fmt.Sprintf("nginx-%s", monster.Name),
			},
			Ports: []corev1.ServicePort{
				{
					Name:	   "http",
					Port:       8000,
					TargetPort: intstr.FromInt(8000),
				},
			},
			Type: corev1.ServiceTypeClusterIP,
		},
	}

	// Retry creating or updating the service
	err := retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingService corev1.Service
		err := r.Get(ctx, client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
		}, &existingService)

		if err != nil && client.IgnoreNotFound(err) != nil {
			log.FromContext(ctx).Error(err, "unable to fetch Service for Monster")
			return err
		}

		if err == nil {
			existingService.Spec = service.Spec
			return r.Update(ctx, &existingService)
		}

		return r.Create(ctx, service)
	})

	if err != nil {
		log.FromContext(ctx).Error(err, "unable to create or update Service for Monster")
	}

	return err
}

func (r *MonsterReconciler) deleteService(ctx context.Context, name, namespace string) error {
	log := log.FromContext(ctx)

	name = fmt.Sprintf("nginx-%s", name) // Prepend 'nginx-' to match service naming

	// Fetch the service
	service := &corev1.Service{}
	if err := r.Get(ctx, client.ObjectKey{Namespace: namespace, Name: name}, service); err != nil {
		if client.IgnoreNotFound(err) != nil {
			log.Error(err, "unable to fetch Service for Monster", "name", name)
			return err
		}
		log.Info("Service already deleted or not found", "name", name)
		return nil
	}

	// Delete the service
	if err := r.Delete(ctx, service); err != nil {
		log.Error(err, "unable to delete Service for Monster", "name", name)
		return err
	}

	log.Info("Successfully deleted Service for Monster", "name", name)
	return nil
}
