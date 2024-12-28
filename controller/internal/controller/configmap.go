package controller

import (
	"fmt"
	"context"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/client-go/util/retry"
	client "sigs.k8s.io/controller-runtime/pkg/client"
	kaschaeferv1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	log "sigs.k8s.io/controller-runtime/pkg/log"
)

// createOrUpdateConfigMap ensures the ConfigMap is created or updated
func (r *MonsterReconciler) createOrUpdateConfigMap(ctx context.Context, monster kaschaeferv1.Monster) error {
	// Skip updates if Monster is being deleted
	if monster.DeletionTimestamp != nil {
		log.FromContext(ctx).Info("Skipping ConfigMap update as Monster is being deleted", "name", monster.Name)
		return nil
	}
	
	// Generate index.html and nginx.conf content as before
	indexHTML := generateHTMLContent(monster) // Use the function from utils.go
	nginxConf := generateNginxConfig(monster)

	// Create or Update the ConfigMap for the monster
	cm := &corev1.ConfigMap{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("monster-%s", monster.Name),
			Namespace: "monsters",
		},
		Data: map[string]string{
			"index.html": indexHTML,
			"nginx.conf": nginxConf,
		},
	}

	// Retry creating or updating the ConfigMap
	err := retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingCM corev1.ConfigMap
		err := r.Get(ctx, client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("monster-%s", monster.Name),
		}, &existingCM)

		if err != nil && client.IgnoreNotFound(err) != nil {
			return err
		}

		if err == nil {
			// Update the existing ConfigMap
			existingCM.Data = cm.Data
			return r.Update(ctx, &existingCM)
		}

		// Create a new ConfigMap if not found
		return r.Create(ctx, cm)
	})

	if err != nil {
		log.FromContext(ctx).Error(err, "unable to create or update ConfigMap for Monster")
	}

	return err
}
