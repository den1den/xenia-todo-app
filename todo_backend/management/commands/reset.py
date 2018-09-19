from django.contrib.auth.models import User
from django.core.management import call_command
from django.core.management.base import BaseCommand

from todo_backend.models import WemosDevice, TodoModel, Location


class Command(BaseCommand):
    def add_arguments(self, parser):
        pass

    def handle(self, *args, **options):
        # Reset
        call_command('migrate', migration_name='zero', app_label='todo_backend')
        call_command('migrate', migration_name='zero', app_label='auth')

        call_command('makemigrations')
        call_command('migrate')

        User.objects.create_superuser('admin', email='den_1_den@hotmail.com', password='admin1234')
        # admin, c = User.objects.get_or_create(username='dennis')
        # admin.email = 'den_1_den@hotmail.com'
        # admin.set_password('dennis1234')
        # admin.save()

        anywhere = Location.objects.create(name='Anywhere')
        l1 = Location.objects.create(name='De deur')
        l2 = Location.objects.create(name='Kelder')
        w1 = WemosDevice.objects.create(number=1, location=l1)
        w2 = WemosDevice.objects.create(number=2, location=l2)

        TodoModel.objects.create(name='Do dat ene ding',
                                 location=w2.location,
                                 completion_date=TodoModel.COMPLETION_DATES[0][0],
                                 priority=TodoModel.PRIORITIES[0][0],
                                 status=TodoModel.STATUSSES[1][0], )
        TodoModel.objects.create(name='Do dat overal ding',
                                 location=anywhere,
                                 completion_date=TodoModel.COMPLETION_DATES[0][0],
                                 priority=TodoModel.PRIORITIES[1][0],
                                 status=TodoModel.STATUSSES[1][0], )
        TodoModel.objects.create(name='Do dat al gedaan ding',
                                 location=w2.location,
                                 completion_date=TodoModel.COMPLETION_DATES[0][0],
                                 priority=TodoModel.PRIORITIES[0][0],
                                 status=TodoModel.STATUSSES[2][0], )
        TodoModel.objects.create(name='Do dat niet meer nodig ding',
                                 location=w2.location,
                                 completion_date=TodoModel.COMPLETION_DATES[0][0],
                                 priority=TodoModel.PRIORITIES[0][0],
                                 status=TodoModel.STATUSSES[0][0], )
