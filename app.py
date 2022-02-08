from flask import Flask, jsonify, request
import pymongo
from datetime import datetime
import threading
import time
import json
import os
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn import metrics
from sklearn.neural_network import MLPRegressor

app = Flask(__name__)

# DB_URL = os.environ.get("DB_URL")
# print(DB_URL)
client = pymongo.MongoClient("mongodb+srv://joaopaulo:87194584ms@cluster-predicao.p0wqe.mongodb.net/database-predicao?retryWrites=true&w=majority",serverSelectionTimeoutMS=5000)



db = client['database-predicao']
sensores = db['sensores_experimento_2']

data = []
qtd_data = 0
TIME_TRAINING_5_MINUTOS = 300
TIME_TRAINING_1_HORA = 3600
TIME_TRAINING_6_HORAS = 21600

# Setting the date in a string var...
datetime_format = "%d/%m/%Y %H:%M"
date_read = datetime.now()
date_time_last = str(date_read.strftime(datetime_format))

# {'activation': 'tanh',
#  'hidden_layer_sizes': (150,),
#  'learning_rate_init': 0.001,
#  'max_iter': 10000}

model_1 = MLPRegressor( activation = "tanh", hidden_layer_sizes=(150,), learning_rate_init=0.001, max_iter=10000) # modelo temperatura

# 'activation': 'logistic',
#  'hidden_layer_sizes': (150,),
#  'learning_rate_init': 0.001,
#  'max_iter': 1000

model_2 = MLPRegressor( activation="logistic", hidden_layer_sizes=(150,),learning_rate_init=0.001, max_iter=1000) # modelo humidade

predictions_1 = []
predictions_2 = []

X_train_1 = None
X_test_1 = None
y_train_1 = None
y_test_1 = None
X_train_2 = None
X_test_2 = None
y_train_2 = None
y_test_2 = None


@app.route('/', methods=['GET'])
def index():
    print(client.server_info())
    return {'hello': 'world',}


@app.route('/dados', methods=['GET'])
def recuperar_dados_da_predicao():
    # TODO IMPLEMENTAR METODO PARA RECUPERAR DADOS DO BANCO
    global model_1, model_2, y_test_1, predictions_1, y_test_2, predictions_2

    now = datetime.now()
    dt = str(now.strftime('%d/%m/%Y %H:%M'))
    hora = now.hour*60
    t = hora+now.minute

    temperature = pd.Series(model_1.predict(np.array([[t]]))).to_json(
        orient='values').replace('[', '').replace(']', '')
    humidity = pd.Series(model_2.predict(np.array([[t]]))).to_json(
        orient='values').replace('[', '').replace(']', '')

    predictions_1 = model_1.predict(X_test_1)
    predictions_2 = model_2.predict(X_test_2)
    print(predictions_1)
    print(predictions_2)

    output = {
        'datetime': dt,
        'data_predict': {
            'temperature': {
                'value': round(float(temperature), 2),
                'MAE': round(metrics.mean_absolute_error(y_test_1, predictions_1), 2),
                'MSE': round(metrics.mean_squared_error(y_test_1, predictions_1), 2),
                'RMSE': round(np.sqrt(metrics.mean_squared_error(y_test_1, predictions_1)), 2)
            },
            'humidity': {
                'value': round(float(humidity), 2),
                'MAE': round(metrics.mean_absolute_error(y_test_2, predictions_2), 2),
                'MSE': round(metrics.mean_squared_error(y_test_2, predictions_2), 2),
                'RMSE': round(np.sqrt(metrics.mean_squared_error(y_test_2, predictions_2)), 2)
            }
        },
    }

    return jsonify(output)
    
# pega dados do mongo e colocar num array "data"
# formato do obj a ser colocado no array :
# {
#     datetime: 300 (hora em minutos + minutos),
#     temperature: 25,
#     humidity: 75
# }
# após colocar, incrementa 1 na quantidade "qtd_data = len(data)"

def get_data_initial():
    global data, qtd_data
    for i in sensores.find():
        datetime_in_string = i['datetime']
        dt = datetime.strptime(datetime_in_string, datetime_format)
        hora = dt.hour*60
        t = hora+dt.minute
        data.append(
            {'datetime': t, 'temperature': i['sensors']['0']['value'], 'humidity': i['sensors']['1']['value']})
    qtd_data = len(data)
    print("Base de dados inserida")


def training_initial():
    global model_1, model_2, data, X_train_1, X_test_1, y_train_1, y_test_1, X_train_2, X_test_2, y_train_2, y_test_2
    get_data_initial()
    print("Training initial...")
    df = pd.DataFrame(data) # transfor em um dataframe para poder aplicar o MLP

    # o que tem 1 é relacionado a temperatura, 2 é relacionado a humidade
    
        # relação hora X temperatura
    X_train_1, X_test_1, y_train_1, y_test_1 = train_test_split(df.drop(
        columns=['temperature', 'humidity']), df['temperature'], random_state=1)
        # relação hora X humidade
    X_train_2, X_test_2, y_train_2, y_test_2 = train_test_split(df.drop(
        columns=['temperature', 'humidity']), df['humidity'], random_state=1)

        # prepara o treino utilizando os dados ja salvos no banco
    model_1.fit(X_train_1, y_train_1)
    model_2.fit(X_train_2, y_train_2)

    print("initial models created")



def training():
    global model_1, model_2, data
    while True:
        time.sleep(TIME_TRAINING_1_HORA)
        print("Training...")
        dataNew = get_data() # novo dado que foi adicionado
        if(len(dataNew) == 0):
            print("nothing new for training")
        else:
            print("partial fit for new data")
            df = pd.DataFrame(dataNew)
            model_1.partial_fit(
                df.drop(columns=['temperature', 'humidity']), df['temperature'])
            model_2.partial_fit(
                df.drop(columns=['temperature', 'humidity']), df['humidity'])



def get_data():
    global date_read, date_time_last, data, qtd_data
    output = []
    print("last datetime training:", date_time_last)
    count = sensores.count_documents({})-qtd_data
    print("qtd new data:", count)
    if(count > 0):
        c = 0

        for i in sensores.find():
            c = c+1 #para pegar os ultimos dados que foram adicionados
            if(c > len(data)):
                  # mesma transformação de objeto anterior
                datetime_in_string = i['datetime']
                dt = datetime.strptime(datetime_in_string, datetime_format)
                hora = dt.hour*60
                t = hora+dt.minute

                output.append(
                    {'datetime': t, 'temperature': i['sensors']['0']['value'], 'humidity': i['sensors']['1']['value']})
                data.append(
                    {'datetime': t, 'temperature': i['sensors']['0']['value'], 'humidity': i['sensors']['1']['value']})
        
        date_read = datetime.now()
        date_time_last = str(date_read.strftime(datetime_format))
    qtd_data = len(data)
    print("qtd data:", len(data))
    return output



def startWebServer():
    port = int(os.environ.get("PORT", 5001))
    app.run(debug=True, host='0.0.0.0', port=port, threaded=True)



if __name__ == "__main__":
    threading.Thread(target=training_initial).start()
    threading.Thread(target=training).start()
    startWebServer()
