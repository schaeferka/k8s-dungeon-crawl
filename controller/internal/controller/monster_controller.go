package controller

import (
	"context"
	"fmt"
	kaschaeferv1 "github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
	appsv1 "k8s.io/api/apps/v1"
	corev1 "k8s.io/api/core/v1"
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/runtime"
	"k8s.io/client-go/util/retry"
	"k8s.io/utils/pointer"
	"k8s.io/apimachinery/pkg/util/intstr"
	networkingv1 "k8s.io/api/networking/v1"
	ctrl "sigs.k8s.io/controller-runtime"
	"sigs.k8s.io/controller-runtime/pkg/client"
	"sigs.k8s.io/controller-runtime/pkg/controller/controllerutil"
	"sigs.k8s.io/controller-runtime/pkg/log"
	"crypto/sha256"
	"encoding/hex"
)

// MonsterReconciler reconciles a Monster object
type MonsterReconciler struct {
	client.Client
	Scheme *runtime.Scheme
}

// +kubebuilder:rbac:groups=kaschaefer.com,resources=monsters,verbs=get;list;watch;create;update;patch;delete
// +kubebuilder:rbac:groups=kaschaefer.com,resources=monsters/status,verbs=get;update;patch
// +kubebuilder:rbac:groups=kaschaefer.com,resources=monsters/finalizers,verbs=update

// Reconcile is part of the main kubernetes reconciliation loop which aims to
// move the current state of the cluster closer to the desired state.
func (r *MonsterReconciler) Reconcile(ctx context.Context, req ctrl.Request) (ctrl.Result, error) {
	_ = log.FromContext(ctx)

	// Fetch the Monster object
	var monster kaschaeferv1.Monster
	if err := r.Get(context.Background(), req.NamespacedName, &monster); err != nil {
		return ctrl.Result{}, client.IgnoreNotFound(err)
	}

	// Check if the monster is being deleted
	if monster.ObjectMeta.DeletionTimestamp != nil {
		if containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
			err := r.handleFinalizerForMonster(&monster)
			if err != nil {
				return ctrl.Result{}, err
			}
			controllerutil.RemoveFinalizer(&monster, "kaschaefer.com/cleanup")
			err = r.Update(context.Background(), &monster)
			if err != nil {
				return ctrl.Result{}, err
			}
		}
		return ctrl.Result{}, nil
	}

	// Add the finalizer if not already present
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		controllerutil.AddFinalizer(&monster, "kaschaefer.com/cleanup")
		err := r.Update(context.Background(), &monster)
		if err != nil {
			return ctrl.Result{}, err
		}
	}

	// Create or update ConfigMap with Monster data
	err := r.createOrUpdateConfigMap(monster)
	if err != nil {
		return ctrl.Result{}, err
	}

	// Create or Update the Service for the monster
	err = r.createOrUpdateService(monster)
	if err != nil {
		return ctrl.Result{}, err
	}

	// Create or Update the Nginx Deployment
	err = r.createOrUpdateDeployment(monster)
	if err != nil {
		return ctrl.Result{}, err
	}

	// Create or Update the Ingress for the monster
	err = r.createOrUpdateIngress(monster)
	if err != nil {
		return ctrl.Result{}, err
	}

	// Return success
	return ctrl.Result{}, nil
}

// createOrUpdateConfigMap ensures the ConfigMap is created or updated
func (r *MonsterReconciler) createOrUpdateConfigMap(monster kaschaeferv1.Monster) error {
	// Generate the index.html content for the monster
	indexHTML := fmt.Sprintf(`
		<!DOCTYPE html>
		<html lang="en">
		<head>
			<meta charset="UTF-8">
			<meta name="viewport" content="width=device-width, initial-scale=1.0">
			<title>Monster Info: %s</title>
			<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/tailwindcss@2.2.19/dist/tailwind.min.css">
		</head>
		<body class="bg-gray-100 text-gray-900 flex flex-col min-h-screen">
			<header class="bg-blue-900 text-white p-4">
				<div class="container mx-auto flex items-center justify-between">
					<h1 class="text-3xl font-bold">K8s Dungeon Crawl</h1>
				</div>
			</header>

			<main class="container mx-auto my-8 flex-grow">
				<div class="bg-white p-6 rounded-lg shadow-lg">
					<h2 class="text-2xl font-bold text-blue-900 mb-4">Monster: %s</h2>
					<ul class="list-disc pl-6 space-y-2">
						<li><strong>ID:</strong> %s</li>
						<li><strong>Type:</strong> %s</li>
						<li><strong>CurrentHP:</strong> %s</li>
						<li><strong>MaxHP:</strong> %s</li>
						<li><strong>Depth:</strong> %s</li>
					</ul>
				</div>
			</main>

			<footer class="bg-blue-900 text-white p-4 text-center">
				<p>&copy; 2024 K8s Dungeon Crawl</p>
			</footer>
		</body>
		</html>
	`, monster.Name, monster.Name, fmt.Sprintf("%d", monster.Spec.ID), monster.Spec.Type, fmt.Sprintf("%d", monster.Spec.CurrentHP), fmt.Sprintf("%d", monster.Spec.MaxHP), fmt.Sprintf("%d", monster.Spec.Depth))

	// Generate the nginx.conf content for the monster
	nginxConf := fmt.Sprintf(`
    worker_processes  auto;

    events {
        worker_connections  1024;
    }

    http {
        include       /etc/nginx/mime.types;
        default_type  application/octet-stream;

        access_log  /var/log/nginx/access.log;

        sendfile        on;
        keepalive_timeout  65;

        include /etc/nginx/conf.d/*.conf;

        server {
            listen       8000;  # Listen on port 8000 for Traefik's web entrypoint (no SSL)
            server_name  "nginx-%s"; 

            # Default location block, serving the index.html from the root
            location / {
                root   /usr/share/nginx/html;
                index  index.html;
            }

            # Location block for handling the monster-specific path
            location /%s {
                root   /usr/share/nginx/html;
                index  index.html;
                try_files $uri $uri/ /index.html;  # Ensure fallback to index.html
            }
        }
    }
`, monster.Name, monster.Name)

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
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("monster-%s", monster.Name),
		}, &existingCM)

		if err != nil && client.IgnoreNotFound(err) != nil {
			return err
		}

		if err == nil {
			// If the ConfigMap exists, update it
			existingCM.Data = cm.Data
			return r.Update(context.Background(), &existingCM)
		}

		// Create new ConfigMap if not found
		return r.Create(context.Background(), cm)
	})

	return err
}


// createOrUpdateService creates or updates the service for the monster's Nginx deployment
func (r *MonsterReconciler) createOrUpdateService(monster kaschaeferv1.Monster) error {
	// Define the service for the monster
	service := &corev1.Service{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
		},
		Spec: corev1.ServiceSpec{
			Selector: map[string]string{
				"app": fmt.Sprintf("nginx-%s", monster.Name),
			},
			Ports: []corev1.ServicePort{
				{
					Name:	   "http",
					Port:       8000,
					TargetPort: intstr.FromInt(8000),
				},
			},
			Type: corev1.ServiceTypeClusterIP, // Use ClusterIP for internal access
		},
	}

	// Retry creating or updating the service
	err := retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingService corev1.Service
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
		}, &existingService)

		if err != nil && client.IgnoreNotFound(err) != nil {
			return err
		}

		if err == nil {
			// If the service exists, update it
			existingService.Spec = service.Spec
			return r.Update(context.Background(), &existingService)
		}

		// Create new service if not found
		return r.Create(context.Background(), service)
	})

	return err
}

// createOrUpdateIngress creates or updates the Ingress for the monster's page
func (r *MonsterReconciler) createOrUpdateIngress(monster kaschaeferv1.Monster) error {
	// Define the Ingress resource for the monster
		// Define the Ingress resource for the monster
		ingress := &networkingv1.Ingress{
			ObjectMeta: metav1.ObjectMeta{
				Name:      fmt.Sprintf("nginx-%s-ingress", monster.Name),
				Namespace: "monsters",
			},
			Spec: networkingv1.IngressSpec{
				IngressClassName: pointer.String("traefik"), // Ingress class for Traefik
				Rules: []networkingv1.IngressRule{
					{
						Host: "localhost", // Access the service via localhost
						IngressRuleValue: networkingv1.IngressRuleValue{
							HTTP: &networkingv1.HTTPIngressRuleValue{
								Paths: []networkingv1.HTTPIngressPath{
									{
										Path:     fmt.Sprintf("/%s", monster.Name), // Dynamic path for each monster
										PathType: func() *networkingv1.PathType {   // Use prefix path matching
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
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s-ingress", monster.Name),
		}, &existingIngress)

		if err != nil && client.IgnoreNotFound(err) != nil {
			return err
		}

		if err == nil {
			// If the Ingress exists, update it
			existingIngress.Spec = ingress.Spec
			return r.Update(context.Background(), &existingIngress)
		}

		// Create new Ingress if not found
		return r.Create(context.Background(), ingress)
	})

	return err
}



// createOrUpdateDeployment ensures the Nginx Deployment is created or updated
func (r *MonsterReconciler) createOrUpdateDeployment(monster kaschaeferv1.Monster) error {
	// Fetch the existing ConfigMap to generate a hash for triggering rolling update
	var cm corev1.ConfigMap
	err := r.Get(context.Background(), client.ObjectKey{
		Namespace: "monsters",
		Name:      fmt.Sprintf("monster-%s", monster.Name),
	}, &cm)
	if err != nil {
		return err
	}

	// Generate a hash for the ConfigMap data to trigger the deployment update when the config changes
	cmHash := fmt.Sprintf("%x", hashData(cm.Data)) // Assuming `hashData` is a function to compute the hash

	// Create or Update the Nginx Deployment
	deployment := &appsv1.Deployment{
		ObjectMeta: metav1.ObjectMeta{
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
			Namespace: "monsters",
			Annotations: map[string]string{
				"configHash": cmHash, // Add a hash of the ConfigMap to trigger rolling update
			},
		},
		Spec: appsv1.DeploymentSpec{
			Replicas: pointer.Int32Ptr(1), // Set to 1 replica
			Selector: &metav1.LabelSelector{
				MatchLabels: map[string]string{
					"app": fmt.Sprintf("nginx-%s", monster.Name),
				},
			},
			Template: corev1.PodTemplateSpec{
				ObjectMeta: metav1.ObjectMeta{
					Labels: map[string]string{
						"app": fmt.Sprintf("nginx-%s", monster.Name),
					},
					Annotations: map[string]string{
						"configHash": cmHash, // Ensure the pod template also has this annotation
					},
				},
				Spec: corev1.PodSpec{
					Containers: []corev1.Container{
						{
							Name:  "nginx",
							Image: "nginx:latest", // Use the latest Nginx image
							Ports: []corev1.ContainerPort{
								{
									ContainerPort: 8000,
								},
							},
							VolumeMounts: []corev1.VolumeMount{
								{
									Name:      "monster-data",
									MountPath: "/usr/share/nginx/html/index.html", // Nginx serves the content from here
									SubPath:   "index.html",                       // Only mount the specific file, not the whole config map
								},
								{
									Name:      "nginx-config",
									MountPath: "/etc/nginx/nginx.conf", // Mount the nginx.conf
									SubPath:   "nginx.conf",             // Only mount the specific nginx config
								},
							},
						},
					},
					Volumes: []corev1.Volume{
						{
							Name: "monster-data",
							VolumeSource: corev1.VolumeSource{
								ConfigMap: &corev1.ConfigMapVolumeSource{
									LocalObjectReference: corev1.LocalObjectReference{
										Name: fmt.Sprintf("monster-%s", monster.Name),
									},
								},
							},
						},
						{
							Name: "nginx-config",
							VolumeSource: corev1.VolumeSource{
								ConfigMap: &corev1.ConfigMapVolumeSource{
									LocalObjectReference: corev1.LocalObjectReference{
										Name: fmt.Sprintf("monster-%s", monster.Name),
									},
								},
							},
						},
					},
				},
			},
			// Add rolling update strategy to ensure pods are updated when ConfigMap changes
			Strategy: appsv1.DeploymentStrategy{
				Type: appsv1.RollingUpdateDeploymentStrategyType,
				RollingUpdate: &appsv1.RollingUpdateDeployment{
					MaxUnavailable: &intstr.IntOrString{IntVal: 1},
					MaxSurge:       &intstr.IntOrString{IntVal: 1},
				},
			},
		},
	}

	// Apply or update the deployment
	err = retry.RetryOnConflict(retry.DefaultRetry, func() error {
		var existingDeployment appsv1.Deployment
		err := r.Get(context.Background(), client.ObjectKey{
			Namespace: "monsters",
			Name:      fmt.Sprintf("nginx-%s", monster.Name),
		}, &existingDeployment)

		if err != nil && client.IgnoreNotFound(err) != nil {
			// Handle error if the deployment doesn't exist
			return err
		}

		if err == nil {
			// If the deployment exists, update it
			existingDeployment.Spec = deployment.Spec
			// Trigger a rolling update of the deployment
			return r.Update(context.Background(), &existingDeployment)
		}

		// Create new deployment if not found
		return r.Create(context.Background(), deployment)
	})

	return err
}

// containsString checks if a string is present in a slice of strings
func containsString(slice []string, s string) bool {
	for _, item := range slice {
		if item == s {
			return true
		}
	}
	return false
}

// Finalizer function to clean up resources when the monster is deleted
func (r *MonsterReconciler) handleFinalizerForMonster(monster *kaschaeferv1.Monster) error {
	// Ensure finalizer is present
	if !containsString(monster.Finalizers, "kaschaefer.com/cleanup") {
		monster.Finalizers = append(monster.Finalizers, "kaschaefer.com/cleanup")
		err := r.Update(context.Background(), monster)
		if err != nil {
			return err
		}
	}

	// Cleanup associated resources (e.g., delete deployment)
	err := r.deleteDeployment(monster.Name, "monsters")
	if err != nil {
		return err
	}

	// Remove finalizer and allow deletion of the object
	monster.Finalizers = removeString(monster.Finalizers, "kaschaefer.com/cleanup")
	err = r.Update(context.Background(), monster)
	if err != nil {
		return err
	}

	return nil
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

func (r *MonsterReconciler) deleteDeployment(name, namespace string) error {
	// Prepend 'nginx-' to the monster name
	name = fmt.Sprintf("nginx-%s", name)
	namespace = "monsters"

	// Delete the Nginx deployment associated with the monster
	deployment := &appsv1.Deployment{}
	err := r.Get(context.Background(), client.ObjectKey{Namespace: namespace, Name: name}, deployment)
	if err != nil {
		return err
	}

	// Delete the deployment
	err = r.Delete(context.Background(), deployment)
	if err != nil {
		return err
	}

	return nil
}

// hashData calculates the hash of the ConfigMap's data and returns it as a string.
func hashData(data map[string]string) string {
	// Combine all the keys and values in the ConfigMap into a single string
	hashInput := ""
	for key, value := range data {
		hashInput += fmt.Sprintf("%s=%s", key, value)
	}

	// Generate a SHA-256 hash of the concatenated string
	hash := sha256.Sum256([]byte(hashInput))
	return hex.EncodeToString(hash[:])
}

// SetupWithManager sets up the controller with the Manager.
func (r *MonsterReconciler) SetupWithManager(mgr ctrl.Manager) error {
	return ctrl.NewControllerManagedBy(mgr).
		For(&kaschaeferv1.Monster{}).
		Owns(&corev1.ConfigMap{}). // Watch for ConfigMap changes as well
		Complete(r)
}
