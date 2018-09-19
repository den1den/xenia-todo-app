from django.db import models
from django.utils import timezone


class WemosDevice(models.Model):
    number = models.IntegerField(unique=True, primary_key=True)
    ip_address = models.GenericIPAddressField(null=True, blank=True)
    location = models.ForeignKey('Location', on_delete=models.CASCADE)
    last_request = models.DateTimeField(null=True)

    def __str__(self):
        return "Wemos device %s (%s)" % (self.number, self.location)


class Location(models.Model):
    name = models.CharField(max_length=200, unique=True)

    def __str__(self):
        return "%s" % (self.name)

    @classmethod
    def everwhere(cls):
        return Location.objects.get(pk=1)


class TodoModel(models.Model):
    COMPLETION_DATES = (
        ('0', 'Unscheduled'),
        ('1', 'Today'),
    )
    PRIORITIES = (
        ('0', 'Unimportant'),
        ('1', 'Important'),
    )
    STATUS_DELETED = '-1'
    STATUS_NORMAL = '0'
    STATUS_DONE = '1'
    STATUSSES = (
        (STATUS_DELETED, 'Deleted'),
        (STATUS_NORMAL, 'Active'),
        (STATUS_DONE, 'Done'),
    )
    name = models.CharField(max_length=16 * 3)
    location = models.ForeignKey('Location', on_delete=models.CASCADE)
    completion_date = models.CharField(choices=COMPLETION_DATES, max_length=1)
    priority = models.CharField(choices=PRIORITIES, max_length=1)
    status = models.CharField(choices=STATUSSES, max_length=2, default=STATUSSES[1][0])
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(null=True, blank=True)

    __original_status = None

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.__original_status = self.status

    def save(self, *args, **kwargs):
        self.updated_at = timezone.now()
        super().save(*args, **kwargs)
        if self.__original_status != self.status:
            change = StatusChange(todo=self, status=self.status)
            change.save()
            self.__original_status = self.status

    @property
    def status_changed_at(self):
        # return datetime.datetime(2017, 1, 1)
        c = StatusChange.objects.filter(todo=self).last()
        return c.date if c else self.created_at

    def __str__(self):
        return 'Todo %s: "%s"' % (self.id, self.name)


class StatusChange(models.Model):
    todo = models.ForeignKey(TodoModel, on_delete=models.CASCADE)
    date = models.DateTimeField(auto_now=True)
    status = models.CharField(choices=TodoModel.STATUSSES, max_length=2)

    class Meta:
        ordering = ('date',)
