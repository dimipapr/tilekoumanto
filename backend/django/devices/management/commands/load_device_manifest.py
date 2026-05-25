import os
import json
from django.core.management.base import BaseCommand
from devices.models import Device

class Command(BaseCommand):
    help = 'Loads provisioned device UUIDs from the certs manifest into the database'

    def handle(self, *args, **options):
        # The path where Docker mounts the certs volume
        manifest_path = "/certs/devices/manifest.json"
        
        if not os.path.exists(manifest_path):
            self.stdout.write(self.style.ERROR(f"❌ Manifest not found at {manifest_path}"))
            self.stdout.write("Did you map the volume correctly in compose.yml?")
            return

        try:
            with open(manifest_path, "r") as f:
                manifest = json.load(f)
        except json.JSONDecodeError:
            self.stdout.write(self.style.ERROR("❌ Failed to parse manifest.json. Invalid JSON."))
            return

        uuids = manifest.get("devices", [])
        if not uuids:
            self.stdout.write(self.style.WARNING("⚠️ Manifest is empty. No devices to load."))
            return

        new_count = 0
        exist_count = 0

        self.stdout.write(f"Found {len(uuids)} devices in manifest. Syncing with database...")

        for uuid_str in uuids:
            # get_or_create safely adds new UUIDs and ignores existing ones
            device, created = Device.objects.get_or_create(uuid=uuid_str)
            if created:
                new_count += 1
                self.stdout.write(self.style.SUCCESS(f"➕ Provisioned new device: {uuid_str}"))
            else:
                exist_count += 1

        self.stdout.write("\n" + "-"*30)
        self.stdout.write(self.style.SUCCESS("✅ Provisioning complete!"))
        self.stdout.write(f"Added:   {new_count}")
        self.stdout.write(f"Skipped: {exist_count} (Already existed)")
        self.stdout.write("-"*30 + "\n")