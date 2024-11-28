from typing import Optional
from pydantic import BaseModel, model_validator, field_validator
from flask import current_app

class Position(BaseModel):
    x: int
    y: int

class Monster(BaseModel):
    id: int
    name: str
    type: str
    hp: int
    max_hp: int
    is_dead: bool
    depth: int
    position: Position
    attack_speed: int
    movement_speed: int
    accuracy: int
    defense: int
    damage_min: int
    damage_max: int
    turns_between_regen: int
    spawn_timestamp: Optional[int] = None
    death_timestamp: Optional[int] = None

    def __init__(self, **data):
        super().__init__(**data)
        current_app.logger.info(f"Monster initialized with data: {data}")
        for key, value in data.items():
            current_app.logger.info(f"Key: {key}, Value: {value}, Type: {type(value)}")

    @classmethod
    @model_validator(mode='before')
    def check_data(cls, values):
        current_app.logger.info(f"Validating monster data (before): {values}")
        for key, value in values.items():
            current_app.logger.info(f"Key: {key}, Value: {value}, Type: {type(value)}")
        return values
    
    @classmethod
    @model_validator(mode='after')
    def check_data_after(cls, values):
        current_app.logger.info(f"Validating monster data (after): {values}")
        current_app.logger.info(f"Monster data after validation: {values}")
        return values

    @classmethod
    @field_validator('is_dead', mode='before')
    def validate_is_dead(cls, value):
        if isinstance(value, int):
            current_app.logger.info(f"Converting is_dead value to boolean: {value}\n")
            value = bool(value)
            current_app.logger.info(f"Converted is_dead value to boolean: {value}\n")
            return bool(value)
        current_app.logger.info(f"Didn't need to convert is_dead value: {value}\n")
        return value

    class Config:
        @staticmethod
        def alias_generator(s: str) -> str:
            return s.lower()

    def to_dict(self):
        return self.model_dump(by_alias=True)