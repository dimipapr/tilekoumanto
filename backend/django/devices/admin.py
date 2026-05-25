from django.contrib import admin
from django.utils import timezone
from datetime import timedelta
from .models import Device, DeviceMessageRaw

@admin.register(Device)
class DeviceAdmin(admin.ModelAdmin):
    list_display = ('uuid', 'connection_status', 'last_seen', 'created_at')
    search_fields = ('uuid',)
    list_filter = ('last_seen', 'created_at')
    readonly_fields = ('uuid', 'created_at', 'last_seen')
    ordering = ('-last_seen',)

    @admin.display(description='Status')
    def connection_status(self, obj):
        # The Admin panel derives the state itself, keeping the model clean
        if not obj.last_seen:
            return "⚫ Unknown"
        if timezone.now() - obj.last_seen < timedelta(minutes=5):
            return "🟢 Online"
        return "🔴 Offline"


@admin.register(DeviceMessageRaw)
class RawMessageAdmin(admin.ModelAdmin):
    list_display = ('device', 'topic', 'received_at')
    list_filter = ('device', 'topic')
    # Makes the JSON payload read-only in the admin panel so you don't accidentally break it
    readonly_fields = ('payload', 'received_at')