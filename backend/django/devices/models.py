#backend/django/devices/models.py

from django.db import models

class Device(models.Model):
    uuid = models.UUIDField(primary_key=True, editable=False)
    created_at = models.DateTimeField(auto_now_add=True)
    last_seen = models.DateTimeField(null=True, blank=True)
    display_name = models.CharField(max_length=100, default="Unnamed Device")

    def __str__(self):
        return getattr(self, "name", "") or str(self.uuid)

class DeviceMessageRaw(models.Model):
    device = models.ForeignKey(Device, on_delete=models.CASCADE, related_name='messages')
    topic = models.CharField(max_length=255)
    device_unix_time_ms = models.BigIntegerField(null=True, blank=True)
    payload = models.JSONField()
    received_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['received_at']
        verbose_name = "Device Message Raw"
        verbose_name_plural = "Device Messages Raw"

    def __str__(self):
        return f"{self.device.uuid} @ {self.received_at}"
    
class PumpStateSample(models.Model):
    device = models.ForeignKey(
        Device,
        on_delete=models.CASCADE,
        related_name="pump_state_samples",
    )
    raw_message = models.OneToOneField(
        DeviceMessageRaw,
        on_delete=models.SET_NULL,
        null=True,
        blank=True,
        related_name="pump_state_sample"
    )

    device_timestamp = models.DateTimeField()
    received_at = models.DateTimeField(auto_now_add=True)

    mains_power_present = models.BooleanField()
    pump_relay_active = models.BooleanField()

    class Meta:
        ordering = ["received_at"]
        indexes = [
            models.Index(fields=["device", "-received_at"]),
        ]
        verbose_name = "Pump State Sample"
        verbose_name_plural = "Pump State Samples"

    def __str__(self):
        return f"{self.device.name} @ {self.received_at}"