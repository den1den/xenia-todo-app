"""todo_backend URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/2.0/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include
from rest_framework.routers import DefaultRouter

from todo_backend.views import TodoViewSet, TodoWemosViewSet, WemosDeviceViewSet, TodoWemosNewViewSet, LocationViewSet, \
    ReviveSet, SoftDeleteTodoViewSet

router = DefaultRouter(trailing_slash=False)
router.register(r'revive', ReviveSet, base_name='revive-all')
router.register(r'all-todos/soft-delete', SoftDeleteTodoViewSet, base_name='todos-soft-delete')
router.register(r'all-todos', TodoViewSet, base_name='todos-all')
router.register(r'all-wemos-devices', WemosDeviceViewSet)
router.register(r'wemos-todos/all', TodoWemosViewSet, base_name='wemos-todos-all')
router.register(r'wemos-todos/new', TodoWemosNewViewSet, base_name='wemos-todos-new')
router.register(r'locations', LocationViewSet, base_name='wemos-todos-new')

urlpatterns = [
    path(r'api/', include(router.urls)),
    path(r'admin/', admin.site.urls),
]
