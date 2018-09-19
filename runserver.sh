#!/bin/sh
cd /home/dennis/xenia-todo
. venv/bin/activate
pip install -r requirements.txt --upgrade
# export DJANGO_SETTINGS_MODULE=todo_backend.settings
# python manage.py collectstatic --noinput
python manage.py runserver 0.0.0.0:8090
