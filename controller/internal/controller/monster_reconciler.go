package controller

import (
	"context"

	v1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	corev1 "k8s.io/api/core/v1"
	"k8s.io/apimachinery/pkg/runtime"
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

	return ctrl.Result{}, nil
}


// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&v1.Monster{}).
		Owns(&corev1.ConfigMap{}). // Watch for ConfigMap changes as well
		Complete(r)
}
