import serial
import time
import matplotlib.pyplot as plt

def monitor_and_plot(port='COM4', baud_rate=115200, duration=20):
    try:
        ser = serial.Serial(port, baud_rate, timeout=1)
        print(f"Conectado em {port} com baud rate {baud_rate}")
    except serial.SerialException as e:
        print(f"Erro ao abrir a porta {port}: {e}")
        return

    # Aguarda o trigger "Ready!" do serial
    print("Aguardando o 'Ready!' do serial...")
    while True:
        if ser.in_waiting:
            try:
                linha = ser.readline().decode('utf-8').strip()
                if "Ready!" in linha:
                    print("Trigger recebido: Ready!")
                    break
            except UnicodeDecodeError:
                print("Erro ao decodificar os dados recebidos")

    # Após o trigger, aguarda os parâmetros no formato "%0.50,1.20,0.02,0.0,0.50%"
    parametros = None
    while parametros is None:
        if ser.in_waiting:
            try:
                linha = ser.readline().decode('utf-8').strip()
                if linha.startswith('%') and linha.endswith('%'):
                    linha_param = linha.strip('%')
                    partes = linha_param.split(',')
                    if len(partes) == 5:
                        try:
                            # Mapeamento: GAIN_SETPOINT, FF, ONE_OVER_Ti, D_GAIN, ANTI_WINDUP_GAIN
                            gain_setpoint = float(partes[0])
                            ff = float(partes[1])
                            one_over_ti = float(partes[2])
                            d_gain = float(partes[3])
                            anti_windup_gain = float(partes[4])
                            parametros = (gain_setpoint, ff, one_over_ti, d_gain, anti_windup_gain)
                            print(f"Parâmetros recebidos: GAIN_SETPOINT={gain_setpoint}, FF={ff}, "
                                  f"ONE_OVER_Ti={one_over_ti}, D_GAIN={d_gain}, "
                                  f"ANTI_WINDUP_GAIN={anti_windup_gain}")
                        except ValueError:
                            print(f"Erro ao converter os parâmetros: {linha}")
                    else:
                        print(f"Formato de parâmetros inválido: {linha}")
            except UnicodeDecodeError:
                print("Erro ao decodificar os dados recebidos")

    # Coleta os dados (em segundos) após receber os parâmetros
    data_points = []  # Lista para armazenar tuplas (timestamp, linha)
    start_time = time.time()
    print(f"Coletando dados por {duration} segundos...")
    try:
        while time.time() - start_time < duration:
            if ser.in_waiting:
                try:
                    linha = ser.readline().decode('utf-8').strip()
                    if linha:
                        timestamp = time.time() - start_time
                        data_points.append((timestamp, linha))
                        print(f"{timestamp:.2f} s: {linha}")
                except UnicodeDecodeError:
                    print("Erro ao decodificar os dados recebidos")
    except KeyboardInterrupt:
        print("Interrompido pelo usuário.")
    finally:
        ser.close()

    lux_referencia = []
    lux_atual = []
    tempos = []

    # Processa cada linha recebida no formato "$x,y$"
    for timestamp, linha in data_points:
        linha = linha.strip('$')
        try:
            valor_ref_str, valor_atual_str = linha.split(',')
            lux_referencia.append(float(valor_ref_str))
            lux_atual.append(float(valor_atual_str))
            tempos.append(timestamp)
        except ValueError:
            print(f"Erro ao fazer parsing da linha: {linha}")

    # Cria o título do gráfico com os parâmetros recebidos
    titulo = (f"Monitor Serial: Lux Atual vs Lux de Referência\n"
              f"GAIN_SETPOINT={parametros[0]:.2f}, FF={parametros[1]:.2f}, "
              f"ONE_OVER_Ti={parametros[2]:.2f}, D_GAIN={parametros[3]:.2f}, "
              f"ANTI_WINDUP_GAIN={parametros[4]:.2f}")

    # Plot dos resultados com eixo X em segundos e limite do eixo Y de 0 a 50
    plt.figure(figsize=(10, 5))
    plt.plot(tempos, lux_atual, 'b-', label='Lux Atual')
    plt.plot(tempos, lux_referencia, 'r--', label='Lux de Referência')
    plt.xlabel('Tempo (s)')
    plt.ylabel('Lux')
    plt.ylim(0, 70)
    plt.title(titulo)
    plt.legend()
    plt.grid(True)
    
    # Define o nome do arquivo com base nos parâmetros recebidos
    filename = f"controlador_{parametros[0]:.2f}_{parametros[1]:.2f}_{parametros[2]:.2f}_{parametros[3]:.2f}_{parametros[4]:.2f}.png"
    plt.savefig(filename)
    print(f"Figura salva como {filename}")
    plt.show()

if __name__ == '__main__':
    monitor_and_plot()
