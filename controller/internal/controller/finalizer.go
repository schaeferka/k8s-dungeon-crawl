package controller

import (
	"context"
	"fmt"
	"github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	appsv1 "k8s.io/api/apps/v1"
	"sigs.k8s.io/controller-runtime/pkg/client"
	log "sigs.k8s.io/controller-runtime/pkg/log"
    "net/http"
    "encoding/json"
    "bytes"
    "k8s.io/client-go/util/retry"
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

    // If the Monster is being deleted, skip adding the finalizer
    if monster.DeletionTimestamp != nil && !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
        log.Info("Skipping finalizer addition as Monster is being deleted", "name", monster.Name)
        return nil
    }

    // Notify the portal of the deletion
    log.Info("Sending deletion notification to the portal", "name", monster.Name)
    if err := sendDeletionNotification(monster.Name, monster.Spec.ID); err != nil {
        log.Error(err, "unable to notify portal of Monster deletion", "name", monster.Name)
        return err
    }

    // Cleanup associated resources (e.g., delete deployment)
    log.Info("Deleting associated Deployment for Monster", "name", monster.Name)
    if err := r.deleteDeployment(ctx, monster.Name, "monsters"); err != nil {
        log.Error(err, "unable to delete Deployment for Monster", "name", monster.Name)
        return err
    }

    // Retry removing the finalizer to handle conflicts
    log.Info("Removing finalizer for Monster", "name", monster.Name)
    return retry.RetryOnConflict(retry.DefaultRetry, func() error {
        latestMonster := &v1.Monster{}
        if err := r.Get(ctx, client.ObjectKey{Name: monster.Name, Namespace: monster.Namespace}, latestMonster); err != nil {
            if client.IgnoreNotFound(err) != nil {
                log.Error(err, "unable to fetch latest Monster for finalizer removal", "name", monster.Name)
                return err
            }
            log.Info("Monster not found, assuming already deleted", "name", monster.Name)
            return nil
        }

        // Remove the finalizer from the latest version
        latestMonster.Finalizers = removeString(latestMonster.Finalizers, "kaschaefer.com/cleanup")
        if err := r.Update(ctx, latestMonster); err != nil {
            log.Error(err, "finalizer unable to remove finalizer from Monster", "name", monster.Name)
            return err
        }

        log.Info("Finalizer successfully removed", "name", monster.Name)
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
