from rest_framework import viewsets, mixins
from rest_framework.generics import get_object_or_404
from rest_framework.viewsets import GenericViewSet

from todo_backend.models import *
from todo_backend.serializers import ToDoReadSerializer, ToDoWriteSerializer, WemosNumberSerializer, \
    WemosDeviceWriteSerializer, \
    LocationSerializer, WemosDeviceReadSerializer, ToDoWemosSerializer


class SoftDeleteTodoViewSet(mixins.DestroyModelMixin, GenericViewSet):
    def perform_destroy(self, instance):
        instance.status = TodoModel.STATUS_DELETED
        instance.save()


class TodoViewSet(viewsets.ModelViewSet):
    """
    Show all todo's in the database
    """
    queryset = TodoModel.objects.all()
    serializer_class = ToDoWriteSerializer

    def get_serializer_class(self):
        if self.request.method == 'GET':
            return ToDoReadSerializer
        return super().get_serializer_class()


class TodoWemosViewSetMixin(GenericViewSet):
    """
    These views require another ?wemos=1 parameter to register the right ip address
    """
    queryset = TodoModel.objects.all()
    serializer_class = ToDoWemosSerializer
    wemos = None

    def get_queryset(self):
        s = WemosNumberSerializer(data=self.request.query_params)
        v = s.is_valid(raise_exception=True)
        self.wemos = WemosDevice.objects.get(number=s.validated_data['number'])
        self.update_ip_of_wemos()
        qs = super().get_queryset().filter(status=TodoModel.STATUS_NORMAL, completion_date='1')
        # To return all todos for every Wemos device uncomment the next line just: return qs
        return qs.filter(location=self.wemos.location).union(
            qs.filter(location=Location.everwhere())
        )

    def update_ip_of_wemos(self):
        old_ip = self.wemos.ip_address
        self.wemos.ip_address = self.request.META['REMOTE_ADDR']
        if old_ip != self.wemos.ip_address:
            self.wemos.save()
            print("Updated wemos %s ip to: %s" % (self.wemos, self.wemos.ip_address))


class TodoWemosViewSet(viewsets.ModelViewSet, TodoWemosViewSetMixin):
    """
    Show all todo's from a certain wemos device (append ?wemos=1 to the URL)
    DELETE does not delete a todo but merely sets it to done
    """

    def get_object(self):
        queryset = self.queryset
        lookup_url_kwarg = self.lookup_url_kwarg or self.lookup_field
        filter_kwargs = {self.lookup_field: self.kwargs[lookup_url_kwarg]}
        obj = get_object_or_404(queryset, **filter_kwargs)
        return obj

    def perform_destroy(self, instance):
        instance.status = TodoModel.STATUS_DONE
        instance.save()


class TodoWemosNewViewSet(mixins.ListModelMixin, TodoWemosViewSetMixin):
    """
    Show all todo's from a certain wemos device, since the last request of this type (append ?wemos=1 to the URL)
    """

    def get_queryset(self):
        request_time = timezone.now()
        q = super().get_queryset()  # Set the appropriate IP and self.wemos
        if self.wemos.last_request is not None:
            q = q.filter(updated_at__gte=self.wemos.last_request)
        self.wemos.last_request = request_time
        return q

    def list(self, request, *args, **kwargs):
        list_response = super().list(request, *args, **kwargs)
        old_time = WemosDevice.objects.get(number=self.wemos.number).last_request
        self.wemos.save()
        print("Updated last timestamp of %s from %s -> %s" % (
            self.wemos, old_time, self.wemos.last_request
        ))
        return list_response


class WemosDeviceViewSet(viewsets.ModelViewSet):
    """
    Show all wemos devices in the database
    """
    queryset = WemosDevice.objects.all()
    serializer_class = WemosDeviceWriteSerializer

    def get_serializer_class(self):
        if self.request.method == 'GET':
            return WemosDeviceReadSerializer
        return super().get_serializer_class()


class LocationViewSet(viewsets.ModelViewSet):
    queryset = Location.objects.all()
    serializer_class = LocationSerializer


class ReviveSet(mixins.ListModelMixin, GenericViewSet):
    queryset = TodoModel.objects.all()
    serializer_class = ToDoReadSerializer

    def list(self, request, *args, **kwargs):
        for t in TodoModel.objects.all():
            t.status = TodoModel.STATUS_NORMAL
            t.save()
        return super().list(request, *args, **kwargs)
