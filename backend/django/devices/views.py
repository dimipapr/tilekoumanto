# backend/django/devices/views.py

from django.http import JsonResponse

def health_check(request):
    # Just return a simple heartbeat. 
    # If the request reaches here, Django is healthy.
    return JsonResponse({"status": "OK"})
