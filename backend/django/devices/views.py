# backend/django/devices/views.py

from django.http import JsonResponse

from .models import Device, PumpStateSample
from .contracts.api import LatestDeviceStateResponse

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
    
    response = LatestDeviceStateResponse(
        device_uuid=device.uuid,
        mains_power_present=latest_sample.mains_power_present,
        pump_relay_active=latest_sample.pump_relay_active,
        device_reported_at=latest_sample.device_timestamp,
        backend_received_at=latest_sample.received_at,
    )
  
    return JsonResponse(response.model_dump(mode="json"))