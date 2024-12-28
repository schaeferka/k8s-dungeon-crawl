/*
Copyright 2024.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package v1

import (
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
)

// MonsterSpec defines the desired state of Monster.
type MonsterSpec struct {
	// Name of the monster.
	Name string `json:"name,omitempty"`

	// Type of the monster (e.g., "bloat", "goblin").
	Type string `json:"type,omitempty"`

	// Unique identifier for the monster.
	ID int `json:"id,omitempty"`

	// Health of the monster.
	CurrentHP int `json:"hp,omitempty"`

	// Maximum health of the monster.
	MaxHP int `json:"maxHp,omitempty"`

	// Level of the monster.
	Depth int `json:"depth,omitempty"`

	// Accuracy of the monster.
	Accuracy int `json:"accuracy,omitempty"`

	// Attack speed of the monster.
	AttackSpeed int `json:"attackSpeed,omitempty"`

	// Maximum damage the monster can inflict.
	DamageMax int `json:"damageMax,omitempty"`

	// Minimum damage the monster can inflict.
	DamageMin int `json:"damageMin,omitempty"`

	// Timestamp of the monster's death.
	DeathTimestamp *metav1.Time `json:"deathTimestamp,omitempty"`

	// Defense of the monster.
	Defense int `json:"defense,omitempty"`

	// Indicates if the monster is dead.
	IsDead bool `json:"isDead,omitempty"`

	// Movement speed of the monster.
	MovementSpeed int `json:"movementSpeed,omitempty"`

	// Position of the monster.
	Position Position `json:"position,omitempty"`

	// Timestamp of the monster's spawn.
	SpawnTimestamp *metav1.Time `json:"spawnTimestamp,omitempty"`

	// Turns between health regeneration.
	TurnsBetweenRegen int `json:"turnsBetweenRegen,omitempty"`
}

// Position defines the coordinates of the monster.
type Position struct {
	X int `json:"x,omitempty"`
	Y int `json:"y,omitempty"`
}

// MonsterStatus defines the observed state of Monster.
type MonsterStatus struct {
	// Phase indicates the current lifecycle phase of the Monster.
	// Possible values: "Initializing", "Active", "Deleting".
	Phase string `json:"phase,omitempty"`
}

// +kubebuilder:object:root=true
// +kubebuilder:subresource:status

// Monster is the Schema for the monsters API.
type Monster struct {
	metav1.TypeMeta   `json:",inline"`
	metav1.ObjectMeta `json:"metadata,omitempty"`

	Spec   MonsterSpec   `json:"spec,omitempty"`
	Status MonsterStatus `json:"status,omitempty"`
}

// +kubebuilder:object:root=true

// MonsterList contains a list of Monster.
type MonsterList struct {
	metav1.TypeMeta `json:",inline"`
	metav1.ListMeta `json:"metadata,omitempty"`
	Items           []Monster `json:"items"`
}

func init() {
	SchemeBuilder.Register(&Monster{}, &MonsterList{})
}
