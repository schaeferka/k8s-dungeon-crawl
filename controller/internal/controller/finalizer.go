package controller

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"net/http"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	"k8s.io/client-go/util/retry"
	"sigs.k8s.io/controller-runtime/pkg/client"
	log "sigs.k8s.io/controller-runtime/pkg/log"
	
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

	// Notify the portal of the deletion
	log.Info("Finalizer - Sending deletion notification to the portal", "name", monster.Name)
	if err := sendDeletionNotification(monster.Name, monster.Spec.ID); err != nil {
		log.Error(err, "Finalizer - unable to notify portal of Monster deletion", "name", monster.Name)
		return err
	}

	// Delete associated Deployment
	log.Info("Finalizer - Deleting associated Deployment for Monster", "name", monster.Name)
	if err := r.deleteDeployment(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Finalizer - unable to delete Deployment for Monster", "name", monster.Name)
		return err
	}

	// Delete associated Service
	log.Info("Finalizer - Deleting associated Service for Monster", "name", monster.Name)
	if err := r.deleteService(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Finalizer - unable to delete Service for Monster", "name", monster.Name)
		return err
	}

	// Remove the finalizer
	log.Info("Finalizer - Removing finalizer from Monster", "name", monster.Name)
	return retry.RetryOnConflict(retry.DefaultRetry, func() error {
		latestMonster := &v1.Monster{}
		if err := r.Get(ctx, client.ObjectKey{Name: monster.Name, Namespace: monster.Namespace}, latestMonster); err != nil {
			if client.IgnoreNotFound(err) != nil {
				log.Error(err, "Finalizer - unable to fetch latest Monster for finalizer removal", "name", monster.Name)
				return err
			}
			log.Info("Finalizer - Monster not found, assuming already deleted", "name", monster.Name)
			return nil
		}

		// Remove the finalizer
		latestMonster.Finalizers = removeString(latestMonster.Finalizers, "kaschaefer.com/cleanup")
		if err := r.Update(ctx, latestMonster); err != nil {
			log.Error(err, "Finalizer - unable to remove finalizer from Monster", "name", monster.Name)
			return err
		}
		log.Info("Finalizer - Finalizer successfully removed", "name", monster.Name)
		return nil
	})
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



