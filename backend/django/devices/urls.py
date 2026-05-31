# backend/django/devices/urls.py

from django.urls import path
from .views import health_check, latest_device_state

urlpatterns = [
    path('health/', health_check, name='api-health'),
    path("devices/<uuid:device_uuid>/state",
         latest_device_state,
         name="latest-device-state",
    ),
]