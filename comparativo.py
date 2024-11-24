import matplotlib.pyplot as plt

with open('servidor_udp/pacotes_lost.txt', 'r') as arquivo:
    linha = arquivo.readline()
    pacotes_perd_udp = linha.strip().split(';')  

pacotes_perd_udp = [float(valor) for valor in pacotes_perd_udp[:-1]]

with open('servidor_tcp/pacotes_lost.txt', 'r') as arquivo:
    linha = arquivo.readline()
    pacotes_perd_tcp = linha.strip().split(';')  

pacotes_perd_tcp = [float(valor) for valor in pacotes_perd_tcp[:-1]]


indice = list(range(len(pacotes_perd_udp)))
plt.plot(indice, pacotes_perd_tcp, marker='o', label = "TCP")
plt.plot(indice, pacotes_perd_udp, marker='o', label="UDP")
plt.xlabel('Teste')
plt.ylabel('Número de Pacotes')
plt.legend()
plt.ylim(0, max(pacotes_perd_udp) + 10)
plt.title('Pacotes Perdidos')
plt.show()



with open('servidor_udp/velocidades.txt', 'r') as arquivo:
    linha = arquivo.readline()
    velocidades_udp = linha.strip().split(';')  

velocidades_udp = [float(valor) for valor in velocidades_udp[:-1]]

with open('servidor_tcp/velocidades.txt', 'r') as arquivo:
    linha = arquivo.readline()
    velocidades_tcp = linha.strip().split(';')  

velocidades_tcp = [float(valor) for valor in velocidades_tcp[:-1]]


indice = list(range(len(pacotes_perd_udp)))
plt.plot(indice, velocidades_tcp, marker='o', label = "TCP")
plt.plot(indice, velocidades_udp, marker='o', label="UDP")
plt.ylim(0, max([max(velocidades_tcp), max(velocidades_udp)]) + 0.1*max(velocidades_udp))
plt.xlabel('Teste')
plt.ylabel('Velocidade de Download (bytes/segundo)')
plt.legend()
plt.title('Comparativo Velocidades de Download')
plt.show()
