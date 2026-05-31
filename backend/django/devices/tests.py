# backend/django/devices/tests.py

from django.test import TestCase

from uuid import uuid4

from django.test import TestCase
from django.urls import reverse
from django.utils import timezone

from .models import Device, DeviceMessageRaw, PumpStateSample

class LatestDeviceStateApiTests(TestCase):
    def make_device(self):
        return Device.objects.create(
            uuid = uuid4(),
            display_name = "Test Device",
        )
    def make_sample(
        self,
        device,
        device_timestamp=None,
        received_at=None,
        mains_power_present=True,
        pump_relay_active=False,
    ):
        sample = PumpStateSample.objects.create(
            device=device,
            device_timestamp=device_timestamp or timezone.now(),
            mains_power_present=mains_power_present,
            pump_relay_active=pump_relay_active,
        )

        if received_at is not None:
            PumpStateSample.objects.filter(pk=sample.pk).update(
                received_at=received_at,
            )
            sample.refresh_from_db()

        return sample
    
    def test_returns_latest_device_state(self):
        device = self.make_device()
        sample = self.make_sample(
            device = device,
            mains_power_present=True,
            pump_relay_active=True,
        )

        response = self.client.get(
            reverse("latest-device-state", args=[device.uuid])
        )

        self.assertEqual(response.status_code, 200)

        body = response.json()
        self.assertEqual(body["device_uuid"], str(device.uuid))
        self.assertEqual(body["mains_power_present"], True)
        self.assertEqual(body["pump_relay_active"], True)
        self.assertEqual(
            body["device_reported_at"],
            sample.device_timestamp.isoformat(),
        )
        self.assertEqual(
            body["backend_received_at"],
            sample.received_at.isoformat(),
        )
    
    def test_unknown_device_returns_404(self):
        response = self.client.get(
            reverse("latest-device-state", args=[uuid4()])
        )

        self.assertEqual(response.status_code, 404)
        self.assertEqual(response.json()["error"], "device_not_found")

    def test_known_device_with_no_samples_returns_404(self):
        device = self.make_device()

        response = self.client.get(
            reverse("latest-device-state", args=[device.uuid])
        )

        self.assertEqual(response.status_code, 404)
        self.assertEqual(response.json()["error"], "device_state_not_found")

    #latest by received_at
    def test_returns_latest_device_state(self):
        device = self.make_device()

        older_sample = self.make_sample(
            device=device,
            mains_power_present=False,
            pump_relay_active=False,
            received_at=timezone.now() - timezone.timedelta(minutes=10),
        )

        latest_sample = self.make_sample(
            device=device,
            mains_power_present=True,
            pump_relay_active=True,
            received_at=timezone.now(),
        )

        response = self.client.get(
            reverse("latest-device-state", args=[device.uuid])
        )

        self.assertEqual(response.status_code, 200)

        body = response.json()

        self.assertEqual(body["device_uuid"], str(device.uuid))
        self.assertEqual(body["mains_power_present"], True)
        self.assertEqual(body["pump_relay_active"], True)
        self.assertEqual(
            body["device_reported_at"],
            latest_sample.device_timestamp.isoformat().replace("+00:00", "Z"),
        )
        self.assertEqual(
            body["backend_received_at"],
            latest_sample.received_at.isoformat().replace("+00:00","Z"),
        )