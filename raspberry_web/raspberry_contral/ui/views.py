from django.shortcuts import render

# Create your views here.
def dashboard(request):
    return render(request, "ui/dashboard.html")

def devices(request):
    return render(request, "ui/devices.html")

def logs(request):
    return render(request, "ui/logs.html")

def settings_view(request):
    return render(request, "ui/settings.html")