package controller

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	"github.com/schaeferka/k8s-dungeon-crawl/dungeon-master/api/v1"
)

// containsString checks if a string is present in a slice of strings
func containsString(slice []string, s string) bool {
	for _, item := range slice {
		if item == s {
			return true
		}
	}
	return false
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

// generateHTMLContent generates the HTML content for the monster page
func generateHTMLContent(monster v1.Monster) string {
	return fmt.Sprintf(`
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
}

// generateNginxConfig generates the Nginx configuration content for the monster
func generateNginxConfig(monster v1.Monster) string {
	return fmt.Sprintf(`
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
}

