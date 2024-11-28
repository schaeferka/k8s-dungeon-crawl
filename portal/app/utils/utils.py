import re
from pydantic import BaseModel
from typing import Any, Dict, Type, get_origin, get_args, Optional
from flask import current_app

def camel_to_snake(name: str) -> str:
    """
    Convert camelCase string to snake_case string.

    Args:
        name (str): The camelCase string.

    Returns:
        str: The snake_case string.
    """
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', name)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()

def convert_value(value: Any, target_type: Type) -> Any:
    """
    Convert a value to the target type, handling common conversions.

    Args:
        value (Any): The value to convert.
        target_type (Type): The target type to convert to.

    Returns:
        Any: The converted value.
    """
    try:
        if target_type == bool:
            if isinstance(value, str):
                return value.lower() in ('true', '1')
            return bool(value)
        elif target_type == int:
            return int(value)
        elif target_type == float:
            return float(value)
        elif target_type == str:
            return str(value)
        elif get_origin(target_type) == list:
            item_type = get_args(target_type)[0]
            return [convert_value(item, item_type) for item in value]
        elif issubclass(target_type, BaseModel):
            return convert_data_to_model(value, target_type)
        return target_type(value)
    except (TypeError, ValueError) as e:
        current_app.logger.error(f"Error converting value '{value}' to {target_type}: {e}")
        return value

def convert_data_to_model(data: Any, model: Type[BaseModel]) -> Any:
    """
    This function takes data and a Pydantic model. It checks each field in the model
    and converts the data to the appropriate type if it doesn't match the model's type.

    Args:
        data (Any): The data received from the game.
        model (Type[BaseModel]): The Pydantic model to which the data should be converted.

    Returns:
        Any: The data with the correct types as defined by the Pydantic model.
    """
    if isinstance(data, list):
        return [convert_data_to_model(item, model) for item in data]
    
    if isinstance(data, dict):
        converted_data = {}
        current_app.logger.info(f"starting convert_data_to_model for: {data}")

        for field, field_type in model.__annotations__.items():
            camel_case_field = camel_to_snake(field)
            if camel_case_field in data:
                value = data[camel_case_field]
                converted_data[field] = convert_value(value, field_type)
            else:
                # If the field is not in the data, leave it as None or its default value
                converted_data[field] = None if field_type != Optional else None
        
        return converted_data
    
    return data