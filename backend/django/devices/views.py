# backend/django/devices/views.py

from datetime import datetime, timezone as dt_timezone

from django.http import JsonResponse
from django.shortcuts import render

from .models import Device, PumpStateSample
from .contracts.api import LatestDeviceStateResponse

def health_check(request):
    # Just return a simple heartbeat. 
    # If the request reaches here, Django is healthy.
    return JsonResponse({"status": "OK"})


def device_list(request):
    devices = Device.objects.order_by("display_name", "uuid")

    rows = []
    for device in devices:
        latest_sample = (
            PumpStateSample.objects
            .filter(device=device)
            .order_by("-received_at")
            .first()
        )

        rows.append(
            {
                "device": device,
                "latest_sample": latest_sample,
            }
        )

    rows.sort(
        key=lambda row: (
            row["latest_sample"].received_at
            if row["latest_sample"]
            else datetime.min.replace(tzinfo=dt_timezone.utc)
        ),
        reverse=True,
    )

    return render(
        request,
        "devices/device_list.html",
        {
            "rows": rows,
        },
    )
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