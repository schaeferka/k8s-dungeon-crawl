package controller

import (
	"context"
	"fmt"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	networkingv1 "k8s.io/api/networking/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/client-go/util/retry"
	"sigs.k8s.io/controller-runtime/pkg/client"
	log "sigs.k8s.io/controller-runtime/pkg/log"
)

// stringPtr returns a pointer to a string value
func stringPtr(s string) *string {
	return &s
}

// createOrUpdateIngress creates or updates the Ingress for the monster's page
func (r *MonsterReconciler) createOrUpdateIngress(ctx context.Context, monster v1.Monster) error {
	// Skip updates if Monster is being deleted
	if monster.DeletionTimestamp != nil {
		log.FromContext(ctx).Info("Skipping Ingress update as Monster is being deleted", "name", monster.Name)
		return nil
	}

	// Define the Ingress resource for the monster
	ingress := &networkingv1.Ingress{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s-ingress", monster.Name),
			Namespace: "monsters",
		},
		Spec: networkingv1.IngressSpec{
			IngressClassName: stringPtr("traefik"), // Ingress class for Traefik
			Rules: []networkingv1.IngressRule{
				{
					Host: "localhost", // Access the service via localhost
					IngressRuleValue: networkingv1.IngressRuleValue{
						HTTP: &networkingv1.HTTPIngressRuleValue{
							Paths: []networkingv1.HTTPIngressPath{
								{
									Path: fmt.Sprintf("/%s", monster.Name), // Dynamic path for each monster
									PathType: func() *networkingv1.PathType { // Use prefix path matching
										pt := networkingv1.PathTypePrefix
										return &pt
									}(),
									Backend: networkingv1.IngressBackend{
										Service: &networkingv1.IngressServiceBackend{
											Name: fmt.Sprintf("nginx-%s", monster.Name), // Service name should match the monster
											Port: networkingv1.ServiceBackendPort{
												Number: 8000, // Port of the nginx service
											},
										},
									},
								},
							},
						},
					},
				},
			},
		},
	}

	// Retry creating or updating the Ingress
	err := retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingIngress networkingv1.Ingress
		err := r.Get(ctx, client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s-ingress", monster.Name),
		}, &existingIngress)

		if err != nil && client.IgnoreNotFound(err) != nil {
			log.FromContext(ctx).Error(err, "unable to fetch Ingress for Monster")
			return err
		}

		if err == nil {
			// If the Ingress exists, update it
			existingIngress.Spec = ingress.Spec
			return r.Update(ctx, &existingIngress)
		}

		// Create new Ingress if not found
		return r.Create(ctx, ingress)
	})

	if err != nil {
		log.FromContext(ctx).Error(err, "unable to create or update Ingress for Monster")
	}

	return err
}

func (r *MonsterReconciler) deleteIngress(ctx context.Context, name, namespace string) error {
	// Prepend 'nginx-' to the monster name for ingress naming
	name = fmt.Sprintf("nginx-%s-ingress", name)

	// Fetch the ingress
	ingress := &networkingv1.Ingress{}
	err := r.Get(ctx, client.ObjectKey{Namespace: namespace, Name: name}, ingress)
	if err != nil {
		if client.IgnoreNotFound(err) == nil {
			// Log and return if the Ingress is not found
			log.FromContext(ctx).Info("Ingress already deleted or not found", "name", name)
			return nil
		}
		// Log and return the error if it's not a NotFound error
		log.FromContext(ctx).Error(err, "unable to fetch Ingress for Monster", "name", name)
		return err
	}

	// Delete the ingress
	err = r.Delete(ctx, ingress)
	if err != nil {
		log.FromContext(ctx).Error(err, "unable to delete Ingress for Monster", "name", name)
		return err
	}

	log.FromContext(ctx).Info("Successfully deleted Ingress for Monster", "name", name)
	return nil
}

