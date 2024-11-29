package controller

import (
	"context"
	"github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	ctrl "sigs.k8s.io/controller-runtime"
	"sigs.k8s.io/controller-runtime/pkg/log"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	"k8s.io/apimachinery/pkg/runtime"
	"sigs.k8s.io/controller-runtime/pkg/client"
	corev1 "k8s.io/api/core/v1"
)

// MonsterReconciler reconciles a Monster object
type MonsterReconciler struct {
	client.Client
	Scheme *runtime.Scheme
}

// Reconcile is part of the main kubernetes reconciliation loop which aims to
// move the current state of the cluster closer to the desired state.
func (r *MonsterReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
	log := log.FromContext(ctx)

	// Fetch the Monster object
	var monster v1.Monster
	if err := r.Get(ctx, req.NamespacedName, &monster); err != nil {
		log.Error(err, "unable to fetch Monster")
		return ctrl.Result{}, client.IgnoreNotFound(err)
	}

	// Handle deletion logic
	if monster.ObjectMeta.DeletionTimestamp != nil {
		if containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
			if err := r.handleFinalizerForMonster(ctx, &monster); err != nil {
				log.Error(err, "unable to handle finalizer for Monster")
				return ctrl.Result{}, err
			}
			controllerutil.RemoveFinalizer(&monster, "kaschaefer.com/cleanup")
			if err := r.Update(ctx, &monster); err != nil {
				log.Error(err, "unable to remove finalizer from Monster")
				return ctrl.Result{}, err
			}
		}
		return ctrl.Result{}, nil
	}

	// Add finalizer if needed
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		controllerutil.AddFinalizer(&monster, "kaschaefer.com/cleanup")
		if err := r.Update(ctx, &monster); err != nil {
			log.Error(err, "unable to add finalizer to Monster")
			return ctrl.Result{}, err
		}
	}

	// Create or Update resources
	if err := r.createOrUpdateConfigMap(ctx, monster); err != nil {
		log.Error(err, "unable to create or update ConfigMap for Monster")
		return ctrl.Result{}, err
	}

	if err := r.createOrUpdateService(ctx, monster); err != nil {
		log.Error(err, "unable to create or update Service for Monster")
		return ctrl.Result{}, err
	}

	if err := r.createOrUpdateDeployment(ctx, monster); err != nil {
		log.Error(err, "unable to create or update Deployment for Monster")
		return ctrl.Result{}, err
	}

	if err := r.createOrUpdateIngress(ctx, monster); err != nil {
		log.Error(err, "unable to create or update Ingress for Monster")
		return ctrl.Result{}, err
	}

	// Return success
	return ctrl.Result{}, nil
}

// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&v1.Monster{}).
		Owns(&corev1.ConfigMap{}). // Watch for ConfigMap changes as well
		Complete(r)
}
