#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <MPU6050_tockn.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h> 

const char *ssid = "Apruma"; // Nome do ponto de acesso WiFi
const char *password = "12345678"; // Senha do ponto de acesso WiFi

#define LED_BUILTIN 2

WebSocketsServer webSocket = WebSocketsServer(81); // Porta WebSocket
WebServer server(80);
MPU6050 mpu6050(Wire);
unsigned long startTime = 0; // Tempo de início da execução

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  // lidar com eventos websocket
}

// Função para piscar a LED por 2 segundos
void blinkForTwoSeconds() {
  // Liga a LED
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100); // Aguarda .1 segundo
  
  // Desliga a LED
  digitalWrite(LED_BUILTIN, LOW);
  delay(100); // Aguarda .1 segundo
}


// Função para salvar os dados em um arquivo TXT na memória interna
void saveData(float x, float y, float z) {
  // Estabele as condições de má postura
  if ((y < -110 || y > -60) || (x > 30 || x < 5)) {
     blinkForTwoSeconds(); // pisca o led
    // Abre o arquivo em modo de escrita, se não existir, cria um novo
    File file = SPIFFS.open("/data.txt", "a");
    if (!file) {
      Serial.println("Falha ao abrir o arquivo!");
      return;
    }

    // Calcula o tempo decorrido desde o início da execução
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;

    // Escreve os dados no arquivo
    file.print("Tempo: ");
    file.print(elapsedTime);
    file.print(" ms, Angulo X: ");
    file.print(x);
    file.print(", Angulo Y: ");
    file.print(y);
    file.print(", Angulo Z: ");
    file.println(z);

    // Fecha o arquivo
    file.close();
  }
}
void setup() {
  
  Serial.begin(115200);
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);

  // Inicializa o sistema de arquivos SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Falha ao inicializar o SPIFFS!");
    
    // Tenta formatar o SPIFFS
    if(SPIFFS.format()){
      Serial.println("SPIFFS formatado com sucesso!");
    } else {
      Serial.println("Falha ao formatar o SPIFFS!");
    }
    return;
  }
  
  // Apagar o arquivo especificado (exemplo: data.txt)
  if (SPIFFS.exists("/data.txt")) {
    if (SPIFFS.remove("/data.txt")) {
      Serial.println("Arquivo data.txt apagado com sucesso");
    } else {
      Serial.println("Falha ao apagar o arquivo data.txt");
    }
  } else {
    Serial.println("Arquivo data.txt não encontrado");
  }



  Serial.println("SPIFFS inicializado com sucesso!");

  // Configura o modo de ponto de acesso WiFi
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endereço IP do ponto de acesso: ");
  Serial.println(IP);

  // Inicia a comunicação I2C
  Wire.begin();
  // Inicia o sensor MPU6050
  mpu6050.begin();

  //inicar o websocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  
  // Define a rota para exibir os dados do sensor
  server.on("/sensor", HTTP_GET, []() {
    // Lê os dados do sensor MPU6050
    mpu6050.update();
    float x = mpu6050.getAngleX();
    float y = mpu6050.getAngleY();
    float z = mpu6050.getAngleZ();

    // Cria uma string com os dados do sensor
    String sensorData = "Angulo X: " + String(x) + "<br>";
    sensorData += "Angulo Y: " + String(y) + "<br>";
    sensorData += "Angulo Z: " + String(z) + "<br>";

    // Envia os dados para o cliente
    server.send(200, "text/html", sensorData);
  });

  // Define a rota para exibir a página HTML
  server.on("/", HTTP_GET, []() {
    // Define o conteúdo da página HTML
    String htmlContent = R"(    <!DOCTYPE html>
    <html lang="pt-BR">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <meta name="description" content="Bem-vindo ao Painel do apruma">
        <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons&display=swap">
        <link rel="icon" href="{{ url_for('static', filename='icon.png') }}" type="image/x-icon">
        <style>
            /* Estilos críticos */
        @font-face {
            font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
            font-weight: normal;
            font-style: normal;
            font-display: swap;
        }
    
        /* Estilos de texto comuns */
        body, h1, h2, span, div {
            font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
        }
    
        /* Cabeçalho */
        header {
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
    
        /* Título principal */
        h1 {
            font-family: -apple-system, sans-serif;
            font-size: 50px;
            color: #666666;
            margin: 0px;
        }
    
        .dark-mode h1 {
            color: #ffffff;
            transition: background-color 0.7s ease, color 0.7s ease;
        }
    
        /* Estilos do corpo da página */
        body {
            background-color: #9f9f9f;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
            transition: background-color 0.7s ease, color 0.s ease;
            text-align: center;
        }
    
        /* Modo escuro */
        body.dark-mode {
            background-color: #000000;
            color: #ffffff;
            transition: background-color 0.7s ease, color 0.7s ease;
        }
    
        /* Padrão de transições */
        .container, .container.dark-mode, .container.dark-mode .login-container {
            transition: background-color 0.9s ease, color 0.9s ease, box-shadow 1s ease;
        }
    
        /* Container principal */
        .container {
            background-color: #dddddd;
            box-shadow: 0 0 100px 2px rgba(0, 122, 255, 0.3);
            border-radius: 16px;
            padding: 20px;
            width: 100%;
            max-width: 320px;
        }
    
        /* Modo escuro para o container */
        .container.dark-mode {
            background-color: #222222;
            color: #ffffff;
        }
    
        .container:hover {
            box-shadow: 0 0 200px 30px rgba(0, 122, 255, 0.3);
        }
    
        .center-container {
            margin: 10px auto;
            top: 0px;
        }
    
        /* Botão de modo escuro */
        .dark-mode-button {
            background-color: #333333;
            color: #ffffff;
            border: none;
            border-radius: 5px;
            font-size: 18px;
            cursor: pointer;
            transition: background-color 0.3s ease, color 0.3s ease;
        }
    
        .dark-mode-button:hover {
            background-color: #444444;
            box-shadow: 0 0 20px #485768;
            transition: box-shadow 0.3s ease;
        }
    
        #login-btn,
        #cadastro-btn,
        #cadastrar-btn {
            background-color: #004a9c;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            font-size: 18px;
            cursor: pointer;
            transition: background-color 0.5s ease;
            text-decoration: none;
            display: inline-block;
            margin-right: 0%;
            margin-top: 0px;
        }
    
        #login-btn:hover {
            background-color: #003776;
            box-shadow: 0 0 15px #003776;
            transition: box-shadow 0.5s ease;
        }
    
        .submit-button {
            background-color: #ff0000;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            font-size: 18px;
            cursor: pointer;
            transition: background-color 0.3s ease, box-shadow 0.5s ease;
        }
        .submit-button:hover {
            background-color: #ae0000;
            box-shadow: 0 0 60px #ae0000;
            transition: background-color 0.5s ease, box-shadow 0.5s ease;
        }
    
        /* Botão de regar a planta */
        .water-button {
            background-color: #004a9c;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            font-size: 18px;
            cursor: pointer;
            transition: background-color 0.3s ease, box-shadow 0.5s ease;
        }
    
        .water-plant {
            margin-top: 20px;
        }
    
        .water-button:hover {
            background-color: #003776;
            box-shadow: 0 0 60px #003776;
            transition: background-color 0.5s ease, box-shadow 0.5s ease;
        }
    
        /* Cabeçalho */
        header {
            text-align: center;
        }
    
        /* Título principal */
        h1 {
            font-family: -apple-system, sans-serif;
            font-size: 50px;
            color: #666666;
            margin: 0px;
        }
    
        section {
            margin-top: 20px;
        }
    
        h2 {
            font-size: 20px;
            color: #666666;
            text-align: left;
        }
        .dark-mode h2 {
            color: #ffffff;
            transition: background-color 0.7s ease, color 0.7s ease;
        }
        /* Itens do sensor */
        .sensor-item {
            display: flex;
            align-items: center;
            margin: 10px 0;
        }
    
        /* Ícones do sensor */
        i.material-icons {
            font-size: 24px;
            margin-right: 10px;
            color: #007aff;
        }
    
        span {
            font-size: 18px;
            color: #777777;
        }
    
        /* Interruptor do darkmode */
        .switch {
            position: relative;
            display: inline-block;
            width: 64px;
            height: 34px;
            margin-right: 12px;
            text-align: right;
        }
    
        /* Alça do interruptor */
        .switch .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #666666;
            transition: 0.6s;
            border-radius: 34px;
        }
    
        /* Ponto de ativação do interruptor */
        .switch .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: #fff;
            transition: 0.6s;
            border-radius: 50%;
            transform: translateX(0);
        }
    
        /* Interruptor ativo */
        .switch input:checked + .slider {
            background-color: #444444;
        }
    
        /* Ponto de ativação do interruptor ativo */
        .switch input:checked + .slider:before {
            background-color: #fff;
            transform: translateX(26px);
        }
    
        .hidden {
            display: none;
        }
    
        /* Ícone de rotação */
        .rotate-icon {
            transform: rotate(180deg);
            transition: transform 0.3s ease;
        }
    
        /* Título do sensor com opção de alternância */
        h2.sensor-toggle.da {
            font-size: 20px;
            color: #666666;
            cursor: pointer;
            position: relative;
            transition: background-color 0.3s ease;
            user-select: none;
        }
    
        h2.sensor-toggle.dark-mode {
            font-size: 20px;
            color: #ffffff;
            cursor: pointer;
            position: relative;
            transition: background-color 0.3s ease;
            user-select: none;
        }
    
    
    
    
    
        /* Ícone de alternância do sensor */
        i#sensor-toggle-icon {
            font-size: 24px;
            margin-right: 10px;
            color: #007aff;
            transition: transform 0.3s ease;
            user-select: none;
        }
    
        /* Aplicar rotação ao ícone quando a classe "rotate-icon" está presente */
        i#sensor-toggle-icon.rotate-icon {
            transform: rotate(180deg);
            user-select: none;
        }
    
        /* Manter a lista oculta quando necessário */
        ul#sensor-list.hidden {
            display: none;
        }
    
        /* Estilos gerais para o input */
        input {
            width: 70%;
            padding: 10px;
            margin: 8px 0;
            box-sizing: border-box;
            border: 1px solid #555;
            border-radius: 5px;
            background-color: #dddddd;
            color: #000;
            text-align: left; /* Alterado para alinhar à esquerda */
        }
    
        /* Estilos para o modo escuro dentro do container específico */
        .container.dark-mode input {
            background-color: #333333;
            color: #ffffff;
        }
    
        .input-group {
            display: flex;
            flex-direction: column;
            align-items: flex-start; /* Alinhar itens à esquerda */
            margin-bottom: 2px; /* Aumentar o espaço entre grupos de input */
        }
    
        .input-group label {
            text-align: left;
            margin-bottom: 2px; /* Aumentar o espaço entre o rótulo e o campo de input */
        }
    
        .input-group input {
            width: 100%;
            padding: 10px;
            box-sizing: border-box;
            border: 1px solid #555;
            border-radius: 5px;
            background-color: #dddddd;
            color: #000;
            text-align: left;
            margin-bottom: 2px; /* Aumentar o espaço entre campos de input */
        }
    
        #time-slider {
            width: 80%;
    
        }
    
        #selected-time {
            margin-top: 5px;
    
        }
    
        .management-section {
            display: block;
            text-align: left;
        }
    
        .botoes-container {
            display: flex;
            justify-content: space-between;  /* Para criar espaço entre os botões */
            margin-top: 5px; 
        }
                /* Add styles for the typing animation */
                .container {
                    width: 100%;
                }
    
                #title {
                    overflow: hidden; /* Hide overflow content */
                    border-right: 2px solid #000; /* Create a blinking cursor effect */
                    white-space: nowrap; /* Prevent text from wrapping */
                    margin: 0;
                    display: inline-block; /* Allow container to resize based on content */
                }
                .container {
                    background-color: #dddddd;
                    box-shadow: 0 0 100px 2px rgba(0, 122, 255, 0.3);
                    border-radius: 16px;
                    padding: 20px;
                    width: 100%;
                    max-width: 200px
                }
        </style>
    </head>
    <body>
        <div>
            <div class="container center-container">
                <header>
                    <h1 id="title">Apruma!</h1>
                </header>
            </div>
    
            <button id="login-btn" onclick="window.location.href='/dashboard'"" class="login-btn">iniciar</button>
        </div>
    </body>
    </html>
    <script>
        // JavaScript for the typing animation
        const title = document.getElementById('title');
        const text = title.innerHTML;
        title.innerText = ''; // Clear the text content
    
        let i = 0;
        function typeWriter() {
            if (i < text.length) {
                title.innerHTML += text.charAt(i);
                i++;
                setTimeout(typeWriter, 300); // Adjust the typing speed (in milliseconds)
            }
        }
        // Trigger the typing animation
        typeWriter();
    </script>
)";
    // Envia a página HTML para o cliente
    server.send(200, "text/html", htmlContent);
  });

  server.on("/dashboard", HTTP_GET, []() {
    // Define o conteúdo da página HTML
    String htmlContent = R"(<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="description" content="Bem-vindo ao Painel do Jardim Inteligente de Vegetais. Monitore e controle os sensores de ar e solo, eletricidade, reservatórios e sensores ambientais do seu jardim de forma prática e eficiente. Regue suas plantas manualmente com um clique. Alterne entre os modos claro e escuro para uma experiência personalizada.">
    <link rel="preload" href="https://fonts.googleapis.com/icon?family=Material+Icons&display=swap" as="style">
    <link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons&display=swap">
    <link rel="icon" href="{{ url_for('static', filename='icon.png') }}" type="image/x-icon"> 
    <script>
        document.addEventListener("DOMContentLoaded", function() {
            var button = document.getElementById("submit");
            button.addEventListener("click", function() {
                // Redireciona para a rota /relatorio
                window.location.href = "/relatorio";
            });
        });
        const webSocket = new WebSocket('ws://' + window.location.hostname + ':81');
        // Quando a conexão WebSocket é aberta
        webSocket.onopen = function(event) {
        console.log('Conectado ao servidor WebSocket.');
        };
           // Quando uma mensagem é recebida do servidor WebSocket
        webSocket.onmessage = function(event) {
        // Parseia os dados recebidos como JSON
        const sensorData = JSON.parse(event.data);

        // Seleciona o container da lista de sensores
        const sensorContainer = document.getElementById('sensor-container');

        // Limpa os dados antigos
        sensorContainer.innerHTML = '';

        // Cria elementos de lista para cada ângulo e adiciona ao container
        const angles = ['Angulo X', 'Angulo Y', 'Angulo Z'];
        angles.forEach(angle => {
            const listItem = document.createElement('li');
            listItem.textContent = `${angle}: ${sensorData[angle]}`;
            sensorContainer.appendChild(listItem);
        });

        // Exibe a lista de sensores
        const sensorList = document.getElementById('sensor-list');
        sensorList.classList.remove('hidden');
        };

        // Em caso de erro na conexão WebSocket
        webSocket.onerror = function(error) {
        console.error('Erro na conexão WebSocket:', error);
        };

        // Em caso de fechamento da conexão WebSocket
        webSocket.onclose = function(event) {
        console.log('Conexão WebSocket fechada.');
        };


        class DarkModeApp {
        constructor() {
            this.initializeDOMElements();
            this.preferredMode = this.getPreferredMode();
            this.init();
        }

        initializeDOMElements() {
            this.myDarkModeToggle = document.getElementById("dark-mode-toggle");
            this.myContainer = document.querySelector(".container");
            this.myBody = document.body;
        }

        init() {
            this.setupEventListeners();
            this.setMyMode(this.preferredMode);
        }

        setupEventListeners() {
            this.myDarkModeToggle.addEventListener("click", () => this.toggleMyDarkMode());
        }

        setMyMode(mode) {
            const isDarkMode = mode === "dark";
            this.myContainer.classList.toggle("dark-mode", isDarkMode);
            this.myBody.classList.toggle("dark-mode", isDarkMode);
            this.updateMyDarkModeToggleText(isDarkMode);
            this.storeMyPreferredMode(mode);
        }

        toggleMyDarkMode() {
            const newMode = this.myBody.classList.contains("dark-mode") ? "light" : "dark";
            this.setMyMode(newMode);
        }

        updateMyDarkModeToggleText(isDarkMode) {
            this.myDarkModeToggle.innerText = isDarkMode ? "Light Mode" : "Dark Mode";
        }

        storeMyPreferredMode(mode) {
            this.setMyCookie("preferredMode", mode);
        }

        getPreferredMode() {
            return this.getMyCookie("preferredMode") || "dark";
        }

        setMyCookie(name, value, days = 365) {
            const date = new Date();
            date.setTime(date.getTime() + days * 24 * 60 * 60 * 1000);
            const expires = "expires=" + date.toUTCString();
            document.cookie = `${name}=${value};${expires};path=/`;
        }

        getMyCookie(name) {
            const cname = name + "=";
            const decodedCookie = decodeURIComponent(document.cookie);
            const ca = decodedCookie.split(';');
            for (let i = 0; i < ca.length; i++) {
                let c = ca[i].trim();
                if (c.indexOf(cname) == 0) {
                    return c.substring(cname.length);
                }
            }
            return "";
        }
}
document.addEventListener("DOMContentLoaded", () => {
    const app = new DarkModeApp();
});

        // Obtém o host e a porta da URL atual
        var currentHost = window.location.hostname;
        
        // Constrói a URL do servidor JSON baseando-se no host e na porta da URL atual
        var apiIpSensor = 'http://' + currentHost + ':' + '8001';
        var apiIpRegar = 'http://' + currentHost + ':' + '8002';
        var apiIpGerenciamento = 'http://' + currentHost + ':' + '8003';
        
        // Exporte as constantes
        const apiUrlSensorData = `${apiIpSensor}/api-sensor-data`;
        const apiUrlWaterPlant = `${apiIpRegar}/api-water-plant`;
        
        class GardenApp {
            constructor() {
                // Elementos do DOM
                this.waterButton = document.getElementById("water-button");
                this.sensorContainer = document.getElementById("sensor-container");
                this.sensorERContainer = document.getElementById("sensorER-container");
                this.sensorERList = document.getElementById("sensorER-list");
                this.sensorsERToggle = document.getElementById("sensorsER");
                this.sensorERToggleIcon = document.getElementById("sensorER-toggle-icon");
                this.sensorESContainer = document.getElementById("sensorES-container");
                this.sensorESList = document.getElementById("sensorES-list");
                this.sensorsESToggle = document.getElementById("sensorsES");
                this.sensorESToggleIcon = document.getElementById("sensorES-toggle-icon");
                this.managementToggleIcon = document.getElementById("Management-toggle-icon")
        
                this.container = document.querySelector(".container");
                this.body = document.body;
        
                // Inicialização
                this.init();
            }
        
            init() {
                this.addEventListeners();
                this.fetchAndUpdateSensorData();
                setInterval(() => this.fetchAndUpdateSensorData(), 2000);
            }
        
            addEventListeners() {
                // Event Listeners
                this.waterButton.addEventListener("click", () => this.waterPlant());
                document.getElementById("sensors").addEventListener("click", () => this.toggleVisibility("sensor-list", "sensor-toggle-icon"));
                document.getElementById("management").addEventListener("click", () => this.toggleVisibility("management-container", "Management-toggle-icon"));
                document.getElementById("sensorsER").addEventListener("click", () => this.toggleVisibility("sensorER-list", "sensorER-toggle-icon"));
                document.getElementById("sensorsES").addEventListener("click", () => this.toggleVisibility("sensorES-list", "sensorES-toggle-icon"));
            }
        
            fetchAndUpdateSensorData() {
                // Busca e Atualização de Dados do Sensor
                fetch(apiUrlSensorData)
                    .then((response) => response.json())
                    .then((data) => {
                        this.updateSensorData(data);
                        this.updateElectricityReservoirData(data);
                        this.updateEnvironmentalSensorsData(data);
                    });
            }
        
            updateSensorData(data) {
                // Atualização dos Dados dos Sensores
                const sensorData = `
                    <div class="sensor-item">
                        <i class="material-icons">thermostat</i>
                        <span>Air Temperature: ${data.air_temp}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">thermostat</i>
                        <span>Soil Temperature: ${data.soil_temp}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">invert_colors</i>
                        <span>Soil pH: ${data.ph}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">wb_sunny</i>
                        <span>Air Humidity: ${data.air_humidity}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">opacity</i>
                        <span>Soil Moisture: ${data.soil_moisture}</span>
                    </div>
                `;
        
                if (!this.sensorContainer.classList.contains("hidden")) {
                    this.sensorContainer.innerHTML = sensorData;
                }
            }
        
            updateElectricityReservoirData(data) {
                // Atualização dos Dados de Eletricidade e Reservatórios
                const sensorERData = `
                    <div class="sensor-item">
                        <i class="material-icons">power</i>
                        <span>Consumption: ${data.electrical_consumption}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">local_drink</i>
                        <span>Reservoir Level 1: ${data.reservoir_l1}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">local_drink</i>
                        <span>Reservoir Level 2: ${data.reservoir_l2}</span>
                    </div>
                `;
        
                if (!this.sensorERList.classList.contains("hidden")) {
                    this.sensorERContainer.innerHTML = sensorERData;
                }
            }
        
            updateEnvironmentalSensorsData(data) {
                // Atualização dos dados de sensores do ambiente
                const sensorESData = `
                    <div class="sensor-item">
                        <i class="material-icons">cloud</i>
                        <span>CO2 Level: ${data.co2}</span>
                    </div>
                    <div class="sensor-item">
                        <i class="material-icons">wb_incandescent</i>
                        <span>Light Level: ${data.light}</span>
                    </div>
                `;
        
                if (!this.sensorESList.classList.contains("hidden")) {
                    this.sensorESContainer.innerHTML = sensorESData;
                }
            }
        
            toggleVisibility(elementId, toggleIconId) {
                const element = document.getElementById(elementId);
                const toggleIcon = document.getElementById(toggleIconId);
            
                if (element && toggleIcon) {
                    element.classList.toggle("hidden");
                    toggleIcon.classList.toggle("rotate-icon");
                }
            }
            
            waterPlant() {
                // Irrigação da Planta
                fetch(apiUrlWaterPlant, {
                    method: "POST",
                })
                .then((response) => response.json())
                .then((data) => {
                    alert(data.message);
                })
                .catch((error) => {
                    console.error("Erro ao enviar a solicitação:", error);
                });
            }
        }
        
        function updateTime() {
            var slider = document.getElementById("time-slider");
            var output = document.getElementById("time-value");
            output.innerHTML = slider.value;
        }
        
        function configurarCampoAutocomplete() {
            const cityInput = document.getElementById("cityInput");
            const suggestionsContainer = document.getElementById("suggestions");
        
            function fetchCitiesFromOpenStreetMap(query) {
                // Use a API de busca do OpenStreetMap diretamente no front-end
                const apiUrl = `${apiIpGerenciamento}/api-search-city?city=${query}&format=json&limit=1`;
            
                return fetch(apiUrl)
                    .then(response => {
                        if (!response.ok) {
                            throw new Error(`Erro na solicitação: ${response.status} - ${response.statusText}`);
                        }
                        return response.json();
                    })
                    .then(data => {
                        console.log("Dados da resposta:", data);
                        return data.map(item => item.display_name);
                    })
                    .catch(error => {
                        console.error("Erro ao obter cidades do OpenStreetMap:", error);
                        throw error; // Propaga o erro para o próximo bloco catch, se necessário
                    });
            }
            
            function showSuggestions(suggestions) {
                suggestionsContainer.innerHTML = "";
            
                if (!Array.isArray(suggestions)) {
                    console.error("As sugestões não são um array:", suggestions);
                    return;
                }
            
                suggestions.forEach(city => {
                    const suggestion = document.createElement("div");
                    suggestion.classList.add("suggestion");
                    suggestion.textContent = city;
                    suggestion.addEventListener("click", () => {
                        cityInput.value = city;
                        suggestionsContainer.innerHTML = "";
                    });
                    suggestionsContainer.appendChild(suggestion);
                });
            }
            
            let timeoutId;
        
            function handleInput() {
                const query = cityInput.value.trim();
        
                if (query.length > 2) {
                    clearTimeout(timeoutId); // Limpa o timeout anterior, se existir
        
                    timeoutId = setTimeout(() => {
                        fetchCitiesFromOpenStreetMap(query)
                            .then(suggestions => showSuggestions(suggestions))
                            .catch(error => console.error("Erro ao obter sugestões:", error));
                    }, 500); // Aguarda 500 milissegundos após a última tecla ser pressionada
                } else {
                    suggestionsContainer.innerHTML = "";
                }
            }
        
            cityInput.addEventListener("input", handleInput);
        }
        
        document.addEventListener("DOMContentLoaded", () => {
            const app = new GardenApp();
            configurarCampoAutocomplete();
        });</script>
    <title>Painel do Jardim</title>
    <style>
        /* Estilos críticos */
    @font-face {
        font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
        font-weight: normal;
        font-style: normal;
        font-display: swap;
    }

    /* Estilos de texto comuns */
    body, h1, h2, span, div {
        font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
    }

    /* Cabeçalho */
    header {
        display: flex;
        justify-content: space-between;
        align-items: center;
    }

    /* Título principal */
    h1 {
        font-family: -apple-system, sans-serif;
        font-size: 50px;
        color: #666666;
        margin: 0px;
    }

    .dark-mode h1 {
        color: #ffffff;
        transition: background-color 0.7s ease, color 0.7s ease;
    }

    /* Estilos do corpo da página */
    body {
        background-color: #9f9f9f;
        margin: 0;
        padding: 0;
        display: flex;
        justify-content: center;
        align-items: center;
        min-height: 100vh;
        font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
        transition: background-color 0.7s ease, color 0.s ease;
        text-align: center;
    }

    /* Modo escuro */
    body.dark-mode {
        background-color: #000000;
        color: #ffffff;
        transition: background-color 0.7s ease, color 0.7s ease;
    }

    /* Padrão de transições */
    .container, .container.dark-mode, .container.dark-mode .login-container {
        transition: background-color 0.9s ease, color 0.9s ease, box-shadow 1s ease;
    }

    /* Container principal */
    .container {
        background-color: #dddddd;
        box-shadow: 0 0 100px 2px rgba(0, 122, 255, 0.3);
        border-radius: 16px;
        padding: 20px;
        width: 100%;
        max-width: 320px;
    }

    /* Modo escuro para o container */
    .container.dark-mode {
        background-color: #222222;
        color: #ffffff;
    }

    .container:hover {
        box-shadow: 0 0 200px 30px rgba(0, 122, 255, 0.3);
    }

    .center-container {
        margin: 10px auto;
        top: 0px;
    }

    /* Botão de modo escuro */
    .dark-mode-button {
        background-color: #333333;
        color: #ffffff;
        border: none;
        border-radius: 5px;
        font-size: 18px;
        cursor: pointer;
        transition: background-color 0.3s ease, color 0.3s ease;
    }

    .dark-mode-button:hover {
        background-color: #444444;
        box-shadow: 0 0 20px #485768;
        transition: box-shadow 0.3s ease;
    }

    #login-btn,
    #cadastro-btn,
    #cadastrar-btn {
        background-color: #004a9c;
        color: white;
        border: none;
        border-radius: 5px;
        padding: 10px 20px;
        font-size: 18px;
        cursor: pointer;
        transition: background-color 0.5s ease;
        text-decoration: none;
        display: inline-block;
        margin-right: 0%;
        margin-top: 0px;
    }

    #login-btn:hover {
        background-color: #003776;
        box-shadow: 0 0 15px #003776;
        transition: box-shadow 0.5s ease;
    }

    .submit-button {
        background-color: #ff0000;
        color: white;
        border: none;
        border-radius: 5px;
        padding: 10px 20px;
        font-size: 18px;
        cursor: pointer;
        transition: background-color 0.3s ease, box-shadow 0.5s ease;
    }
    .submit-button:hover {
        background-color: #ae0000;
        box-shadow: 0 0 60px #ae0000;
        transition: background-color 0.5s ease, box-shadow 0.5s ease;
    }

    /* Botão de regar a planta */
    .water-button {
        background-color: #004a9c;
        color: white;
        border: none;
        border-radius: 5px;
        padding: 10px 20px;
        font-size: 18px;
        cursor: pointer;
        transition: background-color 0.3s ease, box-shadow 0.5s ease;
    }

    .water-plant {
        margin-top: 20px;
    }

    .water-button:hover {
        background-color: #003776;
        box-shadow: 0 0 60px #003776;
        transition: background-color 0.5s ease, box-shadow 0.5s ease;
    }

    /* Cabeçalho */
    header {
        text-align: center;
    }

    /* Título principal */
    h1 {
        font-family: -apple-system, sans-serif;
        font-size: 50px;
        color: #666666;
        margin: 0px;
    }

    section {
        margin-top: 20px;
    }

    h2 {
        font-size: 20px;
        color: #666666;
        text-align: left;
    }
    .dark-mode h2 {
        color: #ffffff;
        transition: background-color 0.7s ease, color 0.7s ease;
    }
    /* Itens do sensor */
    .sensor-item {
        display: flex;
        align-items: center;
        margin: 10px 0;
    }

    /* Ícones do sensor */
    i.material-icons {
        font-size: 24px;
        margin-right: 10px;
        color: #007aff;
    }

    span {
        font-size: 18px;
        color: #777777;
    }

    /* Interruptor do darkmode */
    .switch {
        position: relative;
        display: inline-block;
        width: 64px;
        height: 34px;
        margin-right: 12px;
        text-align: right;
    }

    /* Alça do interruptor */
    .switch .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #666666;
        transition: 0.6s;
        border-radius: 34px;
    }

    /* Ponto de ativação do interruptor */
    .switch .slider:before {
        position: absolute;
        content: "";
        height: 26px;
        width: 26px;
        left: 4px;
        bottom: 4px;
        background-color: #fff;
        transition: 0.6s;
        border-radius: 50%;
        transform: translateX(0);
    }

    /* Interruptor ativo */
    .switch input:checked + .slider {
        background-color: #444444;
    }

    /* Ponto de ativação do interruptor ativo */
    .switch input:checked + .slider:before {
        background-color: #fff;
        transform: translateX(26px);
    }

    .hidden {
        display: none;
    }

    /* Ícone de rotação */
    .rotate-icon {
        transform: rotate(180deg);
        transition: transform 0.3s ease;
    }

    /* Título do sensor com opção de alternância */
    h2.sensor-toggle.da {
        font-size: 20px;
        color: #666666;
        cursor: pointer;
        position: relative;
        transition: background-color 0.3s ease;
        user-select: none;
    }

    h2.sensor-toggle.dark-mode {
        font-size: 20px;
        color: #ffffff;
        cursor: pointer;
        position: relative;
        transition: background-color 0.3s ease;
        user-select: none;
    }





    /* Ícone de alternância do sensor */
    i#sensor-toggle-icon {
        font-size: 24px;
        margin-right: 10px;
        color: #007aff;
        transition: transform 0.3s ease;
        user-select: none;
    }

    /* Aplicar rotação ao ícone quando a classe "rotate-icon" está presente */
    i#sensor-toggle-icon.rotate-icon {
        transform: rotate(180deg);
        user-select: none;
    }

    /* Manter a lista oculta quando necessário */
    ul#sensor-list.hidden {
        display: none;
    }

    /* Estilos gerais para o input */
    input {
        width: 70%;
        padding: 10px;
        margin: 8px 0;
        box-sizing: border-box;
        border: 1px solid #555;
        border-radius: 5px;
        background-color: #dddddd;
        color: #000;
        text-align: left; /* Alterado para alinhar à esquerda */
    }

    /* Estilos para o modo escuro dentro do container específico */
    .container.dark-mode input {
        background-color: #333333;
        color: #ffffff;
    }

    .input-group {
        display: flex;
        flex-direction: column;
        align-items: flex-start; /* Alinhar itens à esquerda */
        margin-bottom: 2px; /* Aumentar o espaço entre grupos de input */
    }

    .input-group label {
        text-align: left;
        margin-bottom: 2px; /* Aumentar o espaço entre o rótulo e o campo de input */
    }

    .input-group input {
        width: 100%;
        padding: 10px;
        box-sizing: border-box;
        border: 1px solid #555;
        border-radius: 5px;
        background-color: #dddddd;
        color: #000;
        text-align: left;
        margin-bottom: 2px; /* Aumentar o espaço entre campos de input */
    }

    #time-slider {
        width: 80%;

    }

    #selected-time {
        margin-top: 5px;

    }

    .management-section {
        display: block;
        text-align: left;
    }

    .botoes-container {
        display: flex;
        justify-content: space-between;  /* Para criar espaço entre os botões */
        margin-top: 5px; 
    }

        /* Estilos críticos */
        body {
            background-color: #9f9f9f;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            font-family: -apple-system, BlinkMacSystemFont, system-ui, "Segoe UI", Roboto, Helvetica, Arial, sans-serif, "Segoe UI Symbol";
            transition: background-color 0.9s ease, color 0.9s ease;
        }

        .hidden {
            display: none;
        }
    </style>
</head>
<body>
    <div class="container center-container">
        <header>
            <h1 id="garden">Apuma</h1>
            <label class="switch">
                <input type="checkbox" id="dark-mode-toggle">
                <span class="slider"></span>
            </label>  
        </header>
        </section>

        <!-- Sensors -->
        <div class="sensor-toggle-container">
            <h2 class="sensor-toggle" id="sensors">
                Sensores
                <i class="material-icons" id="sensor-toggle-icon">keyboard_arrow_down</i>
            </h2>
        </div>
        <!--Dados dos sensores-->
        <ul id="sensor-list" class="hidden">
            <section class="sensor" id="sensor-container">
                <!-- Os dados do sensor são inseridos aqui usando JavaScript -->
            </section>
        </ul>

        <div class="botoes-container">
            <section class="submit">
                <button id="submit" class="submit-button">Relatorio</button>
            </section>
        </div>
    </div>
</body>
</html>
)";
    // Envia a página HTML para o cliente
    server.send(200, "text/html", htmlContent);
  });

  // Define a rota para o relatório
  server.on("/relatorio", HTTP_GET, []() {
    // Abre o arquivo para leitura
    File file = SPIFFS.open("/data.txt", "r");
    if (!file) {
      server.send(404, "text/plain", "Arquivo não encontrado");
      return;
    }

    // Envia o conteúdo do arquivo como resposta
    server.sendHeader("Content-Disposition", "attachment; filename=\"relatorio.txt\"");//cabeçalho para indicar download
    server.streamFile(file, "text/plain");
    file.close();
  });
  // Inicia o servidor
  server.begin();
}

void loop() {
  webSocket.loop();

  // Atualiza os ângulos
  mpu6050.update();
  float x = mpu6050.getAngleX();
  float y = mpu6050.getAngleY();
  float z = mpu6050.getAngleZ();

  // Crie um objeto JSON para armazenar os ângulos

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["Angulo X"] = x;
  jsonDoc["Angulo Y"] = y;
  jsonDoc["Angulo Z"] = z;

  // Serialize o objeto JSON para uma string
  String sensorData;
  serializeJson(jsonDoc, sensorData);

  // Envia os dados via WebSocket para todos os clientes conectados
  webSocket.broadcastTXT(sensorData);
  // Chama a função para salvar os dados
  saveData(x, y, z);
  // Adicione um pequeno atraso, se necessário, para evitar sobrecarregar a conexão
  delay(250); // Ajuste conforme necessário
  server.handleClient();
}
