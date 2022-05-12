import pandas as pd

def dictionary_to_list(dic):
    list = []
    for key in dic:
        list.append(dic[key])
    return list

def parse_timeline():
    df = pd.read_csv("timeline.txt", header=None, names=["marciano", "inicio", "fin"])

    df['duracion'] = df.apply(lambda row : row['fin']-row['inicio'], axis = 1)
    df['marciano'] = df.apply(lambda row : str(int(row['marciano'])), axis = 1)
    # Mantiene solo los marcianos con una duracion mayor a 0
    df = df[df['duracion'] > 0]
    # Obtiene marcianos
    marcianos = df['marciano'].unique().tolist()
    marcianos.sort()
    # Agrega tiempos vacios
    for index, row in df.iterrows():
        if index>0:
            if last_row['fin']!=row['inicio']:
                df.loc[index-0.5]= "", last_row['fin'], row['inicio'], row['inicio']-last_row['fin']
        last_row = row
    df = df.sort_index().reset_index(drop=True)
    # Convierte al formato de lista
    timelines = dictionary_to_list(df.to_dict(orient="index"))

    return (timelines, marcianos)

def parse_marcianos():
    df = pd.read_csv("marcianos.txt", header=None, names=["marciano", "duracion", "arribo", "periodo", "creacion", "fin"])
    mode = df.iloc[0][0]
    algoritmo_interactivo = df.iloc[0][1]
    algoritmo_rtos = df.iloc[0][2]
    df['marciano'] = df.apply(lambda row : str(int(row['marciano'])), axis = 1)
    df['vacio'] = df.apply(lambda row : False, axis = 1)
    df.drop(0,inplace=True)
    # Si es RTOS, agrega repeticiones
    if mode == 1:
        for index, row in df.iterrows():
            periodo = row['periodo']
            ultimoArribo = row['arribo']
            finalizacion = row['fin']
            while finalizacion > ultimoArribo+periodo:
                df.loc[df.index.max() + 1] = row['marciano'], row['duracion'], ultimoArribo+periodo,0,0,0,0
                ultimoArribo+=periodo
    arribos = dictionary_to_list(df.to_dict(orient="index"))

    # Se obtiene el algoritmo
    if mode == 2:
        if algoritmo_interactivo == 1:
            algoritmo = "FCFS"
        elif algoritmo_interactivo == 2:
            algoritmo = "PRIORITY"
        else:
            algoritmo = "SRTN"
    else:
        if algoritmo_rtos == 1:
            algoritmo = "EDF"
        else:
            algoritmo = "RM"

    return (arribos, algoritmo)


