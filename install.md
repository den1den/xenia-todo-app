# Windows
Install python

Then from this directory run:
 - `virtualenv venv`
 - `venv/Scripts/activate.bat`
 - `pip install -r requirements.txt`
 - `python manage.py reset`

Then start the program with:
- `python manage.py runserver`
- You can login on [http://127.0.0.1:7777/admin](http://127.0.0.1:7777/admin) with username `admin` and password `admin1234`
- You can visit the API on `http://127.0.0.1:7777/api/`

If another IP address is used it must be added to the `todo_backend\settings.py`. Replace the `ALLOWED_HOSTS` variable with `ALLOWED_HOSTS=['192.168.1.40']` if the device has ip address `192.168.1.40` for example

