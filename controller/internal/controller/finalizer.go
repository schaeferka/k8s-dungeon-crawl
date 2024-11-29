package controller

import (
	"context"
	"fmt"
	"github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	appsv1 "k8s.io/api/apps/v1"
	"sigs.k8s.io/controller-runtime/pkg/client"
	log "sigs.k8s.io/controller-runtime/pkg/log"
)

// Finalizer function to clean up resources when the monster is deleted
func (r *MonsterReconciler) handleFinalizerForMonster(ctx context.Context, monster *v1.Monster) error {
	// Ensure finalizer is present
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		monster.Finalizers = append(monster.Finalizers, "kaschaefer.com/cleanup")
		if err := r.Update(ctx, monster); err != nil {
			log.FromContext(ctx).Error(err, "unable to add finalizer to Monster")
			return err
		}
	}

	// Cleanup associated resources (e.g., delete deployment)
	if err := r.deleteDeployment(ctx, monster.Name, "monsters"); err != nil {
		log.FromContext(ctx).Error(err, "unable to delete Deployment for Monster")
		return err
	}

	// Remove finalizer and allow deletion of the object
	monster.Finalizers = removeString(monster.Finalizers, "kaschaefer.com/cleanup")
	if err := r.Update(ctx, monster); err != nil {
		log.FromContext(ctx).Error(err, "unable to remove finalizer from Monster")
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

func (r *MonsterReconciler) deleteDeployment(ctx context.Context, name, namespace string) error {
	// Prepend 'nginx-' to the monster name
	name = fmt.Sprintf("nginx-%s", name)

	// Delete the Nginx deployment associated with the monster
	deployment := &appsv1.Deployment{}
	if err := r.Get(ctx, client.ObjectKey{Namespace: namespace, Name: name}, deployment); err != nil {
		log.FromContext(ctx).Error(err, "unable to fetch Deployment for Monster")
		return err
	}

	// Delete the deployment
	if err := r.Delete(ctx, deployment); err != nil {
		log.FromContext(ctx).Error(err, "unable to delete Deployment for Monster")
		return err
	}

	return nil
}
