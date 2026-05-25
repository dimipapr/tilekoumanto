from django.db import models

class Device(models.Model):
    uuid = models.UUIDField(primary_key=True, editable=False)
    created_at = models.DateTimeField(auto_now_add=True)
    last_seen = models.DateTimeField(null=True, blank=True)

    def __str__(self):
        return str(self.uuid)

class DeviceMessageRaw(models.Model):
    device = models.ForeignKey(Device, on_delete=models.CASCADE, related_name='messages')
    topic = models.CharField(max_length=255)
    device_timestamp = models.DateTimeField()
    payload = models.JSONField()
    received_at = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ['-received_at']
        verbose_name = "Device Message Raw"
        verbose_name_plural = "Device Messages Raw"

    def __str__(self):
        return f"{self.device.uuid} @ {self.received_at}"