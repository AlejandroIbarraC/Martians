import pandas as pd

def dictionary_to_list(dic):
    list = []
    for key in dic:
        list.append(dic[key])
    return list

def parse_timeline():
    df = pd.read_csv("src/timeline.txt", header=None, names=["marciano", "inicio", "fin"])

    df['duracion'] = df.apply(lambda row : row['fin']-row['inicio'], axis = 1)
    df['marciano'] = df.apply(lambda row : str(int(row['marciano'])), axis = 1)
    # Mantiene solo los marcianos con una duracion mayor a 0
    df = df[df['duracion'] > 0]
    # Obtiene marcianos
    marcianos = df['marciano'].unique().tolist()
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

# TODO: Que se hace con el periodo?
def parse_marcianos():
    df = pd.read_csv("src/marcianos.txt", header=None, names=["marciano", "duracion", "arribo", "periodo", "creacion"])
    df['marciano'] = df.apply(lambda row : str(int(row['marciano'])), axis = 1)
    df['vacio'] = df.apply(lambda row : False, axis = 1)
    mode = df.iloc[-1][0]
    df.drop(df.tail(1).index,inplace=True)
    arribos = dictionary_to_list(df.to_dict(orient="index"))
    return (arribos, "algoritmo")


