# backend/django/devices/views.py

from django.http import JsonResponse

from .models import Device, PumpStateSample

def health_check(request):
    # Just return a simple heartbeat. 
    # If the request reaches here, Django is healthy.
    return JsonResponse({"status": "OK"})

def latest_device_state(request, device_uuid):
    try:
        device = Device.objects.get(uuid = device_uuid)
    except Device.DoesNotExist:
        return JsonResponse(
            {"error": "device_not_found"},
            status=404,
        )
    
    latest_sample = (
        PumpStateSample.objects
        .filter(device = device)
        .order_by("-received_at")
        .first()
    )

    if latest_sample is None:
        return JsonResponse(
            {"error": "device_state_not_found"},
            status=404,
        )
  
    return JsonResponse(
        {
            "device_uuid":str(device.uuid),
            "mains_power_present": latest_sample.mains_power_present,
            "pump_relay_active": latest_sample.pump_relay_active,
            "device_reported_at": latest_sample.device_timestamp.isoformat(),
            "backend_received_at": latest_sample.received_at.isoformat(),
        }
    )