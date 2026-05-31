# backend/django/devices/urls.py

from django.urls import path
from .views import health_check

urlpatterns = [
    path('health/', health_check, name='api-health'),
]