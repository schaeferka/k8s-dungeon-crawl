package controller

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	log "sigs.k8s.io/controller-runtime/pkg/log"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	apierrors "k8s.io/apimachinery/pkg/api/errors"
)

func sendDeletionNotification(monsterName string, monsterID int) error {
	portalURL := "http://portal-service.portal.svc.cluster.local:5000/monsters/notify-deletion"
	payload := map[string]interface{}{
		"monsterName": monsterName,
		"monsterID":   monsterID,
		"namespace":   "monsters",
	}

	jsonData, err := json.Marshal(payload)
	if err != nil {
		return fmt.Errorf("failed to marshal notification payload: %v", err)
	}

	resp, err := http.Post(portalURL, "application/json", bytes.NewBuffer(jsonData))
	if err != nil {
		return fmt.Errorf("failed to send notification: %v", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("received non-OK response from portal: %v", resp.Status)
	}

	return nil
}

func (r *MonsterReconciler) handleFinalizerForMonster(ctx context.Context, monster *v1.Monster) error {
	log := log.FromContext(ctx)

	// Notify the portal
	log.Info("Finalizer - Sending deletion notification to the portal", "name", monster.Name)
	if err := sendDeletionNotification(monster.Name, monster.Spec.ID); err != nil {
		log.Error(err, "Finalizer - Unable to notify portal", "name", monster.Name)
		// Proceed with other cleanup steps even if notification fails
	}

	// Delete associated Deployment
	log.Info("Finalizer - Deleting associated Deployment for Monster", "name", monster.Name)
	if err := r.deleteDeployment(ctx, monster.Name, "monsters"); err != nil {
		if apierrors.IsNotFound(err) {
			log.Info("Finalizer - Deployment already deleted", "name", monster.Name)
		} else {
			log.Error(err, "Finalizer - Unable to delete Deployment", "name", monster.Name)
			return err
		}
	}

	// Delete associated resources (Service, ConfigMap, etc.)
	log.Info("Finalizer - Deleting associated Service for Monster", "name", monster.Name)
	if err := r.deleteService(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Finalizer - Unable to delete Service", "name", monster.Name)
		return err
	}

	log.Info("Finalizer - Deleting associated Ingress for Monster", "name", monster.Name)
	if err := r.deleteIngress(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Finalizer - Unable to delete Ingress", "name", monster.Name)
		return err
	}

	log.Info("Finalizer - Deleting associated ConfigMap for Monster", "name", monster.Name)
	if err := r.deleteConfigMap(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Finalizer - Unable to delete ConfigMap", "name", monster.Name)
		return err
	}

	// Remove the finalizer to allow the CRD to be deleted
	log.Info("Finalizer - Removing finalizer from Monster", "name", monster.Name)
	controllerutil.RemoveFinalizer(monster, "kaschaefer.com/cleanup")
	if err := r.Update(ctx, monster); err != nil {
		log.Error(err, "Finalizer - Unable to remove finalizer", "name", monster.Name)
		return err
	}

	log.Info("Finalizer - Finalizer successfully removed", "name", monster.Name)
	return nil
}



// removeString removes a string from a slice of strings
/* func removeString(slice []string, s string) []string {
	for i, item := range slice {
		if item == s {
			return append(slice[:i], slice[i+1:]...)
		}
	}
	return slice
} */



