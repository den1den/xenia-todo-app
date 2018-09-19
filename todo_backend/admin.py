from django.contrib import admin

from todo_backend.models import WemosDevice, Location
from .models import TodoModel

admin.site.register(TodoModel)
admin.site.register(WemosDevice)
admin.site.register(Location)
