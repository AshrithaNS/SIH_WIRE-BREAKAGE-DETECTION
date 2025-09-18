!pip install requests scikit-learn pandas matplotlib joblib

import requests, time
import pandas as pd
import numpy as np
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report
import joblib

CHANNEL_ID = "3077643"             # replace with your channel
READ_API_KEY = "EHNZ45V1PQPUVCE9"  # replace with your read key
RESULTS = 200                      # number of entries to pull
POLL_INTERVAL = 10                 # seconds between realtime checks

def fetch_thingspeak(channel_id=CHANNEL_ID, read_key=READ_API_KEY, results=RESULTS):
    url = f"https://api.thingspeak.com/channels/{channel_id}/feeds.json?api_key={read_key}&results={results}"
    r = requests.get(url, timeout=10)
    r.raise_for_status()
    data = r.json()
    df = pd.DataFrame(data['feeds'])
    for c in ['field1','field2','field3','field4']:
        if c in df.columns:
            df[c] = pd.to_numeric(df[c], errors='coerce')
    df['created_at'] = pd.to_datetime(df['created_at'])
    df = df.sort_values('created_at').reset_index(drop=True)
    return df

def build_features(df):
    df = df.copy()
    df.rename(columns={'field1':'vibDigital','field2':'vibAnalog','field3':'voltage','field4':'current'}, inplace=True)
    df[['vibDigital','vibAnalog','voltage','current']] = df[['vibDigital','vibAnalog','voltage','current']].ffill().bfill()
    df['I'] = df['current']
    df['V'] = df['voltage']
    df['dI'] = df['I'].diff().fillna(0)
    df['dV'] = df['V'].diff().fillna(0)
    df['I_mean_3'] = df['I'].rolling(3, min_periods=1).mean()
    df['I_std_3']  = df['I'].rolling(3, min_periods=1).std().fillna(0)
    df['V_mean_3'] = df['V'].rolling(3, min_periods=1).mean()
    df['vibAnalog_mean_3'] = df['vibAnalog'].rolling(3, min_periods=1).mean()
    noise_eps = 0.05
    df['I_low'] = (df['I'].abs() < noise_eps).astype(int)

    df['label'] = ((df['V'] < 1.0) | (df['I'] < 0.05) | (df['vibDigital'] == 0)).astype(int)
    return df

def train_model(df_feat):
    features = ['V','I','dI','dV','I_mean_3','I_std_3','vibAnalog_mean_3','I_low']
    data = df_feat.dropna(subset=features + ['label'])
    X = data[features].values
    y = data['label'].astype(int).values
    X_train, X_test, y_train, y_test = train_test_split(X,y,test_size=0.2,random_state=42,stratify=y)
    clf = RandomForestClassifier(n_estimators=100, random_state=1)
    clf.fit(X_train,y_train)
    y_pred = clf.predict(X_test)
    print("\n=== Model Report ===")
    print(classification_report(y_test,y_pred))
    joblib.dump(clf, 'wire_break_detector.pkl')
    print("Model saved: wire_break_detector.pkl")
    return clf

def predict_latest(clf):
    df_latest = fetch_thingspeak(results=10)
    df_latest = build_features(df_latest)
    row = df_latest.iloc[-1]
    features = ['V','I','dI','dV','I_mean_3','I_std_3','vibAnalog_mean_3','I_low']
    X = row[features].astype(float).values.reshape(1,-1)
    prob = clf.predict_proba(X)[0][1]
    pred = clf.predict(X)[0]
    print(f"[{row['created_at']}] Pred={pred} prob={prob:.3f} V={row['V']:.2f} I={row['I']:.2f}")
    return pred, prob

df = fetch_thingspeak()
df_feat = build_features(df)
print("Fetched rows:", len(df_feat))

clf = train_model(df_feat)

print("\n--- Realtime Prediction Loop ---")
while True:
    try:
        pred, prob = predict_latest(clf)
        if pred == 1:
            print("Wire Broken Detected")
        else:
            print("Wire Normal")
        time.sleep(POLL_INTERVAL)
    except Exception as e:
        print("Error:", e)
        time.sleep(5)
