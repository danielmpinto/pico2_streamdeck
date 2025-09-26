import serial
import subprocess
import os
import platform

# Diretório do projeto (onde está este script)
project_root = os.path.dirname(os.path.abspath(__file__))
build_dir = os.path.join(project_root, "build")

# Caminhos das ferramentas
if platform.system() == "Windows":
    ninja_path = os.path.expandvars(r"%USERPROFILE%\.pico-sdk\ninja\v1.12.1\ninja.exe")
    picotool_path = os.path.expandvars(r"%USERPROFILE%\.pico-sdk\picotool\2.1.1\picotool\picotool.exe")
    openocd_path = os.path.expandvars(r"%USERPROFILE%\.pico-sdk\openocd\0.12.0+dev\openocd.exe")
else:
    ninja_path = os.path.expanduser("~/.pico-sdk/ninja/v1.12.1/ninja")
    picotool_path = os.path.expanduser("~/.pico-sdk/picotool/2.1.1/picotool/picotool")
    openocd_path = os.path.expanduser("~/.pico-sdk/openocd/0.12.0+dev/openocd")

# Abre porta USB
ser = serial.Serial('/dev/ttyACM1', 115200, timeout=1)

print("Escutando a porta /dev/ttyACM1...")

def run_compile():
    os.makedirs(build_dir, exist_ok=True)
    if not os.path.exists(os.path.join(build_dir, "build.ninja")):
        print("Configuração inicial com CMake...")
        subprocess.run(["cmake", ".."], cwd=build_dir, check=True)

    print("Compilando com Ninja...")
    result = subprocess.run(
        [ninja_path, "-C", build_dir],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        msg = f"ERRO:\n{result.stderr[:200]}"  # envia só os primeiros 200 caracteres
        print(msg)
        ser.write(msg.encode())   # manda para o Pico
    else:
        ok_msg = "COMPILADO OK\n"
        print(ok_msg)
        ser.write(ok_msg.encode())
        
def run_play():
    uf2_path = os.path.join(build_dir, "PicoDock_TFT_Resistive_Example.uf2")
    print("Rodando projeto (picotool load)...")
    subprocess.run([picotool_path, "load", uf2_path, "-fx"], check=True)

import pyautogui

def run_debug():
    print("Enviando F5 (Debug)...")
    pyautogui.press("f5")

def run_step():
    print("Enviando F10 (Step Over)...")
    pyautogui.press("f10")


def run_stop():
    print("Enviando Shift+F5 (Stop Debug)...")
    pyautogui.hotkey("shift", "f5")


while True:
    try:
        line = ser.readline().decode(errors="ignore").strip()
        if not line:
            continue

        print(f"Recebido: {line}")
        cmd = line.upper()

        if cmd == "COMPILAR":
            run_compile()
        elif cmd == "PLAY":
            run_play()
        elif cmd == "STEP":
            run_step()
        elif cmd == "DEBUG":
            run_debug()
        elif cmd == "STOP":
            run_stop()
        else:
            print(f"Comando desconhecido: {cmd}")

    except KeyboardInterrupt:
        print("Encerrando...")
        break
    except subprocess.CalledProcessError as e:
        print(f"Erro ao executar comando: {e}")

