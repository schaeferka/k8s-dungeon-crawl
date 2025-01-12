package controller

import (
	"context"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	corev1 "k8s.io/api/core/v1"
	appsv1 "k8s.io/api/apps/v1"
	"k8s.io/apimachinery/pkg/runtime"
	apierrors "k8s.io/apimachinery/pkg/api/errors"
	networkingv1 "k8s.io/api/networking/v1"
	"fmt"
	ctrl "sigs.k8s.io/controller-runtime"
	"sigs.k8s.io/controller-runtime/pkg/client"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	"sigs.k8s.io/controller-runtime/pkg/controller"
	"sigs.k8s.io/controller-runtime/pkg/builder"
	"k8s.io/client-go/util/workqueue"
	"time"
	"sigs.k8s.io/controller-runtime/pkg/log"
	"k8s.io/apimachinery/pkg/types"
	"sigs.k8s.io/controller-runtime/pkg/handler"
	"sigs.k8s.io/controller-runtime/pkg/predicate"
)

// MonsterReconciler reconciles a Monster object
type MonsterReconciler struct {
	client.Client
	Scheme *runtime.Scheme
}

func (r *MonsterReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
	log := log.FromContext(ctx)

	// Fetch the Monster object
	var monster v1.Monster
	if err := r.Get(ctx, req.NamespacedName, &monster); err != nil {
		if client.IgnoreNotFound(err) != nil {
			log.Error(err, "Reconcile - unable to fetch Monster")
		} else {
			log.Info("Reconcile - Monster not found, might have been deleted", "name", req.NamespacedName)
		}
		return ctrl.Result{}, client.IgnoreNotFound(err)
	}

	// Handle deletion logic
	if monster.DeletionTimestamp != nil {
		log.Info("Reconcile - Monster is marked for deletion", "name", monster.Name)
		if containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
			// Execute finalizer logic
			if err := r.handleFinalizerForMonster(ctx, &monster); err != nil {
				log.Error(err, "Reconcile - unable to handle finalizer for Monster", "name", monster.Name)
				return ctrl.Result{}, err
			}
		}
		// Skip further processing for deletion
		return ctrl.Result{}, nil
	}

	// Ensure the status.phase field is initialized
	if monster.Status.Phase == "" {
		log.Info("Reconcile - Initializing status.phase", "name", monster.Name)
		monster.Status.Phase = "Initializing"
		if err := r.Status().Update(ctx, &monster); err != nil {
			log.Error(err, "Reconcile - unable to set status.phase to Initializing", "name", monster.Name)
			return ctrl.Result{}, err
		}
		// Allow update to take effect before further processing
		return ctrl.Result{}, nil
	}

	// Add finalizer if needed
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		log.Info("Reconcile - Adding finalizer", "name", monster.Name)
		controllerutil.AddFinalizer(&monster, "kaschaefer.com/cleanup")
		if err := r.Update(ctx, &monster); err != nil {
			log.Error(err, "Reconcile - unable to add finalizer", "name", monster.Name)
			return ctrl.Result{}, err
		}
		// Allow finalizer addition to take effect before further processing
		return ctrl.Result{}, nil
	}

	// Handle cleanup logic only for Active Monsters
	if monster.Status.Phase == "Active" {
		var deployment appsv1.Deployment
		deploymentName := fmt.Sprintf("nginx-%s", monster.Name)
		if err := r.Get(ctx, client.ObjectKey{Namespace: "monsters", Name: deploymentName}, &deployment); err != nil {
			if apierrors.IsNotFound(err) {
				log.Info("Reconcile - Deployment missing, triggering cleanup", "name", deploymentName)
				if cleanupErr := r.cleanupMissingDeployment(ctx, &monster); cleanupErr != nil {
					log.Error(cleanupErr, "Reconcile - unable to clean up resources after Deployment deletion", "name", monster.Name)
					return ctrl.Result{}, cleanupErr
				}
				// Stop further reconciliation as the Monster will be cleaned up
				return ctrl.Result{}, nil
			} else {
				log.Error(err, "Reconcile - error fetching Deployment", "name", deploymentName)
				return ctrl.Result{}, err
			}
		}
	} 


	// Handle regular reconciliation logic (create/update resources)
	if err := r.createOrUpdateConfigMap(ctx, monster); err != nil {
		log.Error(err, "Reconcile - unable to create or update ConfigMap", "name", monster.Name)
		return ctrl.Result{}, err
	}

	if err := r.createOrUpdateService(ctx, monster); err != nil {
		log.Error(err, "Reconcile - unable to create or update Service", "name", monster.Name)
		return ctrl.Result{}, err
	}

	if err := r.createOrUpdateDeployment(ctx, monster); err != nil {
		log.Error(err, "Reconcile - unable to create or update Deployment", "name", monster.Name)
		return ctrl.Result{}, err
	}

	if err := r.createOrUpdateIngress(ctx, monster); err != nil {
		log.Error(err, "Reconcile - unable to create or update Ingress", "name", monster.Name)
		return ctrl.Result{}, err
	}

	// Transition Monster to Active after successful initialization
	if monster.Status.Phase == "Initializing" {
		log.Info("Reconcile - Setting status.phase to Active", "name", monster.Name)
		monster.Status.Phase = "Active"
		if err := r.Status().Update(ctx, &monster); err != nil {
			log.Error(err, "Reconcile - unable to set status.phase to Active", "name", monster.Name)
			return ctrl.Result{}, err
		}
	}

	log.Info("Reconcile - Reconciliation completed successfully", "name", monster.Name)
	return ctrl.Result{}, nil
}


// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
	WithOptions(controller.Options{
		RateLimiter: workqueue.NewTypedItemExponentialFailureRateLimiter[ctrl.Request](5*time.Millisecond, 1000*time.Second), // Rate limiter
	}).
		For(&v1.Monster{}). // Watch for changes to Monster resources
		Watches(
			&appsv1.Deployment{},
			handler.EnqueueRequestsFromMapFunc(func(ctx context.Context, obj client.Object) []ctrl.Request {
				deployment, ok := obj.(*appsv1.Deployment)
				if !ok {
					return nil
				}

				// Extract the Monster name from the Deployment labels
				monsterName, found := deployment.Labels["owner"]
				if !found || deployment.Namespace != "monsters" {
					return nil
				}

				// Return a request to reconcile the corresponding Monster
				return []ctrl.Request{
					{
						NamespacedName: types.NamespacedName{
							Name:      monsterName,
							Namespace: "dungeon-master-system",
						},
					},
				}
			}),
			builder.WithPredicates(predicate.ResourceVersionChangedPredicate{}), // Only reconcile if resource version changes
		).
		Owns(&corev1.ConfigMap{}). // Watch ConfigMaps created by this controller
		Owns(&corev1.Service{}).   // Watch Services created by this controller
		Owns(&networkingv1.Ingress{}). // Watch Ingresses created by this controller
		Complete(r)
}



func (r *MonsterReconciler) cleanupMissingDeployment(ctx context.Context, monster *v1.Monster) error {
	log := log.FromContext(ctx)

	// Ensure cleanup only applies to Active Monsters
	if (monster.Status.Phase != "Active") {
		log.Info("Cleanup - Skipping cleanup for non-Active Monster", "name", monster.Name)
		return nil
	}

	// Notify the portal
	log.Info("Cleanup - Notifying portal about deleted Deployment", "name", monster.Name)
	if err := sendDeletionNotification(monster.Name, monster.Spec.ID, monster.Spec.Depth); err != nil {
		log.Error(err, "Cleanup - unable to notify portal", "name", monster.Name)
		return err
	}

	// Delete associated resources in the monsters namespace
	log.Info("Cleanup - Deleting associated resources in monsters namespace", "name", monster.Name)
	if err := r.deleteService(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Cleanup - unable to delete Service for Monster", "name", monster.Name)
		return err
	}
	if err := r.deleteIngress(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Cleanup - unable to delete Ingress for Monster", "name", monster.Name)
		return err
	}
	if err := r.deleteConfigMap(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Cleanup - unable to delete ConfigMap for Monster", "name", monster.Name)
		return err
	}

	// Delete the Monster CRD in dungeon-master-system namespace
	log.Info("Cleanup - Deleting Monster CRD", "name", monster.Name)
	if err := r.Delete(ctx, monster); err != nil {
		log.Error(err, "Cleanup - unable to delete Monster CRD", "name", monster.Name)
		return err
	}

	log.Info("Cleanup - Successfully cleaned up resources for Monster", "name", monster.Name)
	return nil
}

