package controller

import (
	"context"
	"github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	ctrl "sigs.k8s.io/controller-runtime"
    "sigs.k8s.io/controller-runtime/pkg/log"
    "k8s.io/client-go/util/retry"
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

func (r *MonsterReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
    log := log.FromContext(ctx)

    // Fetch the Monster object
    var monster v1.Monster
    if err := r.Get(ctx, req.NamespacedName, &monster); err != nil {
        if client.IgnoreNotFound(err) != nil {
            log.Error(err, "unable to fetch Monster")
        } else {
            log.Info("Monster not found, might have been deleted", "name", req.NamespacedName)
        }
        return ctrl.Result{}, client.IgnoreNotFound(err)
    }

    // Handle deletion logic
    if monster.ObjectMeta.DeletionTimestamp != nil {
        if containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
            log.Info("Handling finalizer for Monster", "name", monster.Name)

            // Handle finalizer logic
            if err := r.handleFinalizerForMonster(ctx, &monster); err != nil {
                log.Error(err, "unable to handle finalizer for Monster", "name", monster.Name)
                return ctrl.Result{}, err
            }

			// Retry removing the finalizer to handle conflicts
			log.Info("Removing finalizer for Monster", "name", monster.Name)
			err := retry.RetryOnConflict(retry.DefaultRetry, func() error {
				latestMonster := &v1.Monster{}
				if err := r.Get(ctx, client.ObjectKey{Name: monster.Name, Namespace: monster.Namespace}, latestMonster); err != nil {
					if client.IgnoreNotFound(err) != nil {
						log.Error(err, "unable to fetch latest Monster for finalizer removal", "name", monster.Name)
						return err
					}
					log.Info("Monster not found, assuming already deleted", "name", monster.Name)
					return nil
				}

				// Check if the finalizer is still present
				if !containsString(latestMonster.Finalizers, "kaschaefer.com/cleanup") {
					log.Info("Finalizer already removed or Monster deleted", "name", monster.Name)
					return nil
				}

				// Remove the finalizer from the latest version
				latestMonster.Finalizers = removeString(latestMonster.Finalizers, "kaschaefer.com/cleanup")
				if err := r.Update(ctx, latestMonster); err != nil {
					//log.Error(err, "reconciler unable to remove finalizer from Monster", "name", monster.Name)
					return err
				}

				log.Info("Finalizer successfully removed", "name", monster.Name)
				return nil
            })
            if err != nil {
                return ctrl.Result{}, err
            }
            return ctrl.Result{}, nil
        }
        return ctrl.Result{}, nil
    }

    // Add finalizer if needed
    if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
        log.Info("Adding finalizer to Monster", "name", monster.Name)
        controllerutil.AddFinalizer(&monster, "kaschaefer.com/cleanup")
        if err := r.Update(ctx, &monster); err != nil {
            log.Error(err, "unable to add finalizer to Monster", "name", monster.Name)
            return ctrl.Result{}, err
        }
    }

    // Create or Update resources
    if err := r.createOrUpdateConfigMap(ctx, monster); err != nil {
        log.Error(err, "unable to create or update ConfigMap for Monster", "name", monster.Name)
        return ctrl.Result{}, err
    }

    if err := r.createOrUpdateService(ctx, monster); err != nil {
        log.Error(err, "unable to create or update Service for Monster", "name", monster.Name)
        return ctrl.Result{}, err
    }

    if err := r.createOrUpdateDeployment(ctx, monster); err != nil {
        log.Error(err, "unable to create or update Deployment for Monster", "name", monster.Name)
        return ctrl.Result{}, err
    }

    if err := r.createOrUpdateIngress(ctx, monster); err != nil {
        log.Error(err, "unable to create or update Ingress for Monster", "name", monster.Name)
        return ctrl.Result{}, err
    }

    // Return success
    //log.Info("Successfully reconciled Monster", "name", monster.Name)
    return ctrl.Result{}, nil
}




// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&v1.Monster{}).
		Owns(&corev1.ConfigMap{}). // Watch for ConfigMap changes as well
		Complete(r)
}