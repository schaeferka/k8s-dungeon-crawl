# K8s Dungeon Crawl

https://github.com/user-attachments/assets/ae13b259-fa0b-4161-b454-37eb7a2d2faf

_Yes, you really can search the Dungeons of Doom for the Amulet of Yendor from your Grafana dashboard_

## Introduction
Welcome to **K8s Dungeon Crawl**, a thrilling adventure that combines the excitement of rogue-like dungeon crawling with the power of cloud-native technologies. This project is a playful experiment that bridges the worlds of gaming and Kubernetes, showing how modern infrastructure can be adapted to novel use cases.

At its core, K8s Dungeon Crawl uses a Kubernetes cluster to simulate and manage game elements, such as monsters and players, while integrating with tools like Grafana and Prometheus for visualization and metrics. It showcases Kubernetes custom resources, controllers, and validating and mutating webhooks.

Key features of the project:
- **Game-Oriented Custom Resources**: Monsters and other in-game elements are represented as Kubernetes resources.
- **Dynamic Event Handling**: Game events, like monster creation and destruction, trigger automated infrastructure changes.
- **Observability**: Prometheus and Grafana provide insights into the game’s state, displaying metrics such as player health, dungeon level, and monster counts.
- **Cloud-Native Tools**: Integrations with NGINX and Kubernetes controllers make this a fully cloud-native experience.

## Getting Started

### Prerequisites
To embark on this adventure, ensure you have the following installed:
- **Kubernetes Cluster**: Tested with `k3d`.
- **kubectl**: For Kubernetes commands
- **Docker**: For building and running container images.
- **Kubebuilder**: To manage the custom controller.

### Installation Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/schaeferka/k8s-dungeon-crawl.git
   cd k8s-dungeon-crawl
   ```

2. Deploy the Kubernetes cluster and core components: 

   ```bash
   npm i
   ```

   ```bash
   npm run start
   ```

### Running the Project

To start the entire project, you can use the following command:
```bash
npm run start
```
This command performs the following steps:

1. Reset the Cluster: Deletes any existing cluster named k8s-dungeon-crawl and creates a new one with the same name.
2. Build and Import Images: Builds Docker images for the game, portal, and controller, and imports them into the cluster.
3. Deploy Resources: Deploys the game, portal, Grafana, Prometheus, and controller resources to the cluster.

### Deployment Process
Here is a breakdown of what each script does in the deployment process:

#### _Cluster Actions_:

**scripts/cluster-action.js**

Manages the creation, deletion, starting, and stopping of the Kubernetes cluster using k3d.

#### _Namespace Actions_:

**scripts/namespace-action.js** 

Handles the creation, deletion, and resetting of Kubernetes namespaces.

#### _Image Actions_:

**scripts/image-action.js** 

Builds and imports Docker images for the game, portal, and controller.

#### _Deploy Resources_:

**scripts/deploy-k8s-resources.sh:** Deploys Kubernetes resources for the game, portal, Grafana, and Prometheus. Sets up needed port-forwarding.

**scripts/deploy-controller.sh**: Deploys the custom controller and its associated resources. Set up needed port-forwarding.

#### _Environment Loading_:

**scripts/load-env.js** 
Loads environment variables from the .env file and optionally prints them.

#### _Port Checking_:

**scripts/check-ports.sh** 

Checks if the required ports are available before starting the deployment.

### Accessing Services

After running npm run start, you can access the following services:

noVNC Viewer: [http://localhost:8090](http://localhost:8090)

Grafana Dashboard: [http://localhost:3000](http://localhost:3000)

Prometheus UI: [http://localhost:9090](http://localhost:9090)

Portal Dashboard: [http://localhost:5000](http://localhost:5000)

Portal Metrics: [http://localhost:5000/metrics](http://localhost:5000/metrics)


## Architecture Overview

### Core Components

#### **1. The Game Server**
The Brogue game server runs inside a container, and its visual output is accessible via noVNC. Player metrics (e.g., health, dungeon level, gold count) are sent to the Portal, which processes and exposes them as metrics.

#### **2. The Portal**
The Portal serves as the central API and event processor:
- **Metrics Exporter**: Exposes game metrics (e.g., player health, monster counts) for Prometheus to scrape.
- **Event Relay**: Sends game events (e.g., monster creation/destruction) to the monster controller.

Also included are web pages to help you monitor what's happening in the game.

#### **3. Custom Resources and Controller**
The `Monster` custom resource represents monsters in the game. A Kubebuilder-based controller listens to changes in these resources and manages corresponding Kubernetes resources, such as NGINX deployments for each monster.

#### **3. Prometheus and Grafana**
- **Prometheus**: Scrapes metrics exposed by the Portal.
- **Grafana**: Visualizes game state metrics on a custom dashboard.

### Event Flow
1. **Monster Creation**:
   - The game server generates send a POST message to the Portal.
   - The Portal creates a CRD to represent the monster.
   - The Kubernetes controller then creates an NGINX deployment with customized content for the index page with information about the monster.

2. **Monster Destruction**:
   - When a monster is killed in the game, the Portal is notified.
   - The Portal then deletes the CRD for the monster.
   - The Monster controller then deletes the corresponding NGINX deployment.

### Event Flow Diagram



### Key CRD: Monster
```yaml
apiVersion: kaschaefer.com/v1
kind: Monster
metadata:
  name: goblin
spec:
  type: melee
  health: 100
```
The Monster CRD defines the schema for monsters, including their type and health.

### Custom Controller Logic
The controller:
- Watches the `Monster` CRD for changes.
- Creates or deletes NGINX deployments based on monster events.
- Generates ConfigMaps for NGINX `index.html` files, displaying monster-specific information. These pages can be easily accessed from the Monsters page of the Portal Dashboard at [http://localhost:5000/monsters](https://localhost:5000/monsters) 

## Observability

#### Grafana Dashboard
- Player information such as health and gold are displayed as live metrics.
- Dungeon level and monster counts are updated in real-time.
- Cluster metrics are displayed.
- A noVNC panel allows direct interaction with the game. Yes, you really can play Brogue from your Grafana dashboard!

#### Prometheus Metrics
The Portal exposes metrics in the following format:
```json
{
  "gold": 50,
  "health": 100,
  "dungeon_level": 2,
  "monster_count": 5
  /// And many more metrics...
}
```
Prometheus scrapes this endpoint and makes the metrics available for Grafana.

## Contributions
Contributions are welcome! If you have ideas to expand the project or fix bugs, feel free to open an issue or submit a pull request.

## License
This project is licensed under the Apache License 2.0. See the LICENSE file for details.

---

Let the dungeon crawl begin!

