from django.conf import settings
from django.contrib.auth.models import User
from rest_framework import authentication
from rest_framework import exceptions


class StupidAuthentication(authentication.BaseAuthentication):
    def authenticate(self, request):
        p = request.META.get('HTTP_AUTHORIZATION')
        if p == settings.STUPID_AUTH_PASSWORD:
            return User(), None
        raise exceptions.AuthenticationFailed()
