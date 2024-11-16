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
	MaxHP int `json:"maxHP,omitempty"`

	// Level of the monster.
	Level int `json:"level,omitempty"`
}

// MonsterStatus defines the observed state of Monster.
type MonsterStatus struct {
	// Add status fields here to track observed state.
	// For example, current health, last seen location, etc.
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
