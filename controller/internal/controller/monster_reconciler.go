package controller

import (
	"context"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	corev1 "k8s.io/api/core/v1"
	appsv1 "k8s.io/api/apps/v1"
	"k8s.io/apimachinery/pkg/runtime"
	networkingv1 "k8s.io/api/networking/v1"
	"fmt"
	ctrl "sigs.k8s.io/controller-runtime"
	"sigs.k8s.io/controller-runtime/pkg/client"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	"sigs.k8s.io/controller-runtime/pkg/log"
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
		log.Info("Reconcile - Setting status.phase to Initializing", "name", monster.Name)
		monster.Status.Phase = "Initializing"
		if err := r.Status().Update(ctx, &monster); err != nil {
			log.Error(err, "Reconcile - unable to set status.phase to Initializing", "name", monster.Name)
			return ctrl.Result{}, err
		}
		// Skip further processing until phase is set
		return ctrl.Result{}, nil
	}

	// Handle cleanup logic only for Active Monsters
	if monster.Status.Phase == "Active" {
		var deployment appsv1.Deployment
		deploymentName := fmt.Sprintf("nginx-%s", monster.Name)
		if err := r.Get(ctx, client.ObjectKey{Namespace: "monsters", Name: deploymentName}, &deployment); err != nil {
			if client.IgnoreNotFound(err) == nil {
				log.Info("Reconcile - Deployment missing, triggering cleanup", "name", deploymentName)
				if err := r.cleanupMissingDeployment(ctx, &monster); err != nil {
					log.Error(err, "Reconcile - unable to clean up missing Deployment for Monster", "name", monster.Name)
					return ctrl.Result{}, err
				}
				return ctrl.Result{}, nil
			}
			log.Error(err, "Reconcile - error fetching Deployment for Monster", "name", monster.Name)
			return ctrl.Result{}, err
		}
	}	

	// Add finalizer if needed
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		log.Info("Adding finalizer to Monster", "name", monster.Name)
		controllerutil.AddFinalizer(&monster, "kaschaefer.com/cleanup")
		if err := r.Update(ctx, &monster); err != nil {
			log.Error(err, "Reconcile - unable to add finalizer to Monster", "name", monster.Name)
			return ctrl.Result{}, err
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
		For(&v1.Monster{}). // Watch for Monster changes
		Owns(&corev1.ConfigMap{}).
		Owns(&appsv1.Deployment{}).
		Owns(&networkingv1.Ingress{}).
		Complete(r)
}


func (r *MonsterReconciler) cleanupMissingDeployment(ctx context.Context, monster *v1.Monster) error {
	log := log.FromContext(ctx)

	// Ensure cleanup only applies to Active Monsters
	if monster.Status.Phase != "Active" {
		log.Info("Cleanup - Skipping cleanup for non-Active Monster", "name", monster.Name)
		return nil
	}

	// Notify the portal
	log.Info("Cleanup - Notifying portal about missing Deployment", "name", monster.Name)
	if err := sendDeletionNotification(monster.Name, monster.Spec.ID); err != nil {
		log.Error(err, "Cleanup - unable to notify portal", "name", monster.Name)
		return err
	}

	// Delete associated Service
	log.Info("Cleanup - Deleting associated Service for Monster", "name", monster.Name)
	if err := r.deleteService(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Cleanup - unable to delete Service for Monster", "name", monster.Name)
		return err
	}

	// Delete associated Ingress
	log.Info("Cleanup - Deleting associated Ingress for Monster", "name", monster.Name)
	if err := r.deleteIngress(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Cleanup - unable to delete Ingress for Monster", "name", monster.Name)
		return err
	}

	// Delete associated ConfigMap
	log.Info("Cleanup - Deleting associated ConfigMap for Monster", "name", monster.Name)
	if err := r.deleteConfigMap(ctx, monster.Name, "monsters"); err != nil {
		log.Error(err, "Cleanup - unable to delete ConfigMap for Monster", "name", monster.Name)
		return err
	}

	// Delete Monster CRD
	log.Info("Cleanup - Deleting Monster CRD", "name", monster.Name)
	if err := r.Delete(ctx, monster); err != nil {
		log.Error(err, "Cleanup - unable to delete Monster CRD", "name", monster.Name)
		return err
	}

	log.Info("Cleanup - Successfully cleaned up resources for Monster", "name", monster.Name)
	return nil
}

