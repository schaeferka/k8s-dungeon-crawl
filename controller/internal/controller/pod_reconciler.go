package controller

import (
    "context"
    "fmt"
    "net/http"

    corev1 "k8s.io/api/core/v1"
	apierrors "k8s.io/apimachinery/pkg/api/errors"
    "k8s.io/apimachinery/pkg/runtime"
    "k8s.io/client-go/tools/record"
    "sigs.k8s.io/controller-runtime/pkg/builder"
    "sigs.k8s.io/controller-runtime/pkg/client"
    "sigs.k8s.io/controller-runtime/pkg/event"
    "sigs.k8s.io/controller-runtime/pkg/log"
    "sigs.k8s.io/controller-runtime/pkg/predicate"
	"sigs.k8s.io/controller-runtime/pkg/controller"
    ctrl "sigs.k8s.io/controller-runtime"
)

// PodReconciler watches Pods that are not processed yet
type PodReconciler struct {
    client.Client
    Scheme   *runtime.Scheme
    Recorder record.EventRecorder
}

var processedAnnotation = "monsties-controller/processed"

// PodReconciler watches Pods in the 'monsties' namespace:
// - Calls "/add" when first created (unprocessed).
// - Calls "/admin-kill" when the Pod is deleted.


// Define a predicate that allows:
//   - Create/Update events for "unprocessed" Pods in the monsties namespace
//   - Delete events for Pods in the monsties namespace
//   - Skips everything else
var monstiesPodPredicate = predicate.Funcs{
    CreateFunc: func(e event.CreateEvent) bool {
        pod, ok := e.Object.(*corev1.Pod)
        if !ok {
            return false
        }
        if pod.Namespace != "monsties" {
            return false
        }
        // Only proceed if not yet processed
        return pod.Annotations[processedAnnotation] != "true"
    },
    UpdateFunc: func(e event.UpdateEvent) bool {
        newPod, ok := e.ObjectNew.(*corev1.Pod)
        if !ok {
            return false
        }
        if newPod.Namespace != "monsties" {
            return false
        }
        // Only proceed if still not processed
        return newPod.Annotations[processedAnnotation] != "true"
    },
    DeleteFunc: func(e event.DeleteEvent) bool {
        // Let Reconcile see the event if it's in 'monsties',
        // so we can call "admin-kill".
        return e.Object.GetNamespace() == "monsties"
    },
    GenericFunc: func(e event.GenericEvent) bool {
        return false // typically ignore Generic events
    },
}

// +kubebuilder:rbac:groups="",resources=pods,verbs=get;list;watch;update;patch
// +kubebuilder:rbac:groups="",resources=events,verbs=create;patch;update

func (r *PodReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
    logger := log.FromContext(ctx)

    // 1. Attempt to fetch the Pod
    var pod corev1.Pod
    err := r.Client.Get(ctx, req.NamespacedName, &pod)
    if err != nil {
        // If we can't find it, that implies the Pod was deleted
        if apierrors.IsNotFound(err) {
            // 2. The Pod is gone from the cluster
            //    => call "admin-kill" if it's in monsties
            //       (predicate ensured it's monsties, but let's double-check)
            if req.Namespace == "monsties" {
                killEndpoint := fmt.Sprintf(
                    "http://portal-service.portal.svc.cluster.local:5000/monsters/admin-kill/pod/%s", 
                    req.Name)
                logger.Info("Calling admin-kill because Pod was deleted", "endpoint", killEndpoint)
                _, callErr := http.Get(killEndpoint)
                if callErr != nil {
                    logger.Error(callErr, "Failed to call admin-kill on delete")
                    // If you want to re-try or handle differently, do so here
                }
            }
            return ctrl.Result{}, nil
        }
        // If it's some other error, requeue
        return ctrl.Result{}, err
    }

    // 3. We found the Pod (so it’s created/updated). If not processed, call /add
    //    Mark it processed afterwards.
    if pod.Namespace != "monsties" {
        return ctrl.Result{}, nil
    }

    // Check if it's already processed
    if pod.Annotations[processedAnnotation] == "true" {
        // Already processed, do nothing
        return ctrl.Result{}, nil
    }

    // Mark as processed FIRST
    if pod.Annotations == nil {
        pod.Annotations = map[string]string{}
    }
    pod.Annotations[processedAnnotation] = "true"
    if err := r.Update(ctx, &pod); err != nil {
        // If update fails, we might see multiple calls
        logger.Error(err, "Failed to set 'processed' annotation")
        return ctrl.Result{}, err
    }

    // Then call the /add endpoint
    addEndpoint := fmt.Sprintf(
        "http://portal-service.portal.svc.cluster.local:5000/monsties/add/%s",
        pod.Name,
    )
    logger.Info("Calling monsties /add endpoint", "endpoint", addEndpoint)
    if _, err := http.Get(addEndpoint); err != nil {
        logger.Error(err, "Failed to call monsties /add endpoint")
        // If you need to remove the annotation for a retry, do it here
        // or else it won't be re-processed.
        return ctrl.Result{}, err
    }

    // Record an event (optional)
    if r.Recorder != nil {
        r.Recorder.Eventf(&pod, "Normal", "EndpointCalled",
            "Called monsties /add endpoint for Pod %s", pod.Name)
    }

    logger.Info("Successfully processed Pod in monsties namespace", "pod", pod.Name)
    return ctrl.Result{}, nil
}

func (r *PodReconciler) SetupWithManager(mgr ctrl.Manager) error {
    return ctrl.NewControllerManagedBy(mgr).
        For(&corev1.Pod{}, builder.WithPredicates(monstiesPodPredicate)).
        WithOptions(controller.Options{ MaxConcurrentReconciles: 1 }).
        Complete(r)
}


