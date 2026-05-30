from django.contrib import admin
from django.utils import timezone
from datetime import timedelta
from django.db.models import F
from .models import Device, DeviceMessageRaw, PumpStateSample

@admin.register(Device)
class DeviceAdmin(admin.ModelAdmin):
    list_display = ('name','last_seen','connection_status', 'created_at','uuid')
    readonly_fields = ('uuid', 'created_at', 'last_seen')

    @admin.display(description='Status')
    def connection_status(self, obj):
        # The Admin panel derives the state itself, keeping the model clean
        if not obj.last_seen:
            return "⚫ Unknown"
        if timezone.now() - obj.last_seen < timedelta(minutes=5):
            return "🟢 Online"
        return "🔴 Offline"
    def get_queryset(self, request):
        queryset = super().get_queryset(request)
        return queryset.order_by(F("last_seen").desc(nulls_last=True), "name", "uuid")

@admin.register(DeviceMessageRaw)
class RawMessageAdmin(admin.ModelAdmin):
    list_display = ('device', 'topic', 'received_at')
    readonly_fields = ('payload', 'received_at')

@admin.register(PumpStateSample)
class PumpStateSampleAdmin(admin.ModelAdmin):
    list_display = (
        "device",
        "mains_power_present",
        "pump_relay_active",
        "device_timestamp",
        "received_at"
    )
    readonly_fields = (
        "device",
        "raw_message",
        "device_timestamp",
        "received_at",
        "mains_power_present",
        "pump_relay_active",
    )