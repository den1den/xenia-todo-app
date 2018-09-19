from rest_framework import serializers

from todo_backend.models import TodoModel, WemosDevice, Location


class ToDoWemosSerializer(serializers.ModelSerializer):
    class Meta:
        model = TodoModel
        fields = ('id', 'name', 'location', 'completion_date', 'priority', 'status',)


class ToDoWriteSerializer(serializers.ModelSerializer):
    class Meta:
        model = TodoModel
        fields = '__all__'


class ToDoReadSerializer(ToDoWriteSerializer):
    location_name = serializers.CharField(source='location.name')
    status_changed = serializers.DateTimeField(source='status_changed_at')


class WemosNumberSerializer(serializers.ModelSerializer):
    wemos = serializers.IntegerField(source='number')

    class Meta:
        model = WemosDevice
        fields = ('wemos',)


class WemosDeviceWriteSerializer(serializers.ModelSerializer):
    class Meta:
        model = WemosDevice
        fields = '__all__'


class WemosDeviceReadSerializer(WemosDeviceWriteSerializer):
    location_name = serializers.CharField(source='location.name')


class LocationSerializer(serializers.ModelSerializer):
    class Meta:
        model = Location
        fields = '__all__'
