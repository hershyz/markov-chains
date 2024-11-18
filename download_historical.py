import yfinance as yf
import datetime

# target assets
tickers = [
    'NVDA',
    'AAPL',
    'MSFT',
    'AMZN',
    'META',
    'GOOGL',
    'TSLA',
    'AVGO',
    'NFLX',
    'CRM',
    'ORCL',
    'AMD',
    'NOW',
    'TXN',
    'IBM',
    'INTU',
    'QCOM',
    'UBER',
    'PANW',
    'PLTR',
    'KLAC',
    'CRWD',
    'ABNB',
    'FTNT',
    'HPE',
    'DELL',
    'ADBE',
    'TEAM',
    'CFLT',
    'AI',
    'SQ',
    'MDB',
    'ASAN',
    'DDOG',
    'LYFT',
    'SNOW',
    'OKTA',
    'PYPL',
    'TSM'
]

# get timestamps for 3 years ago -> now
end_date = datetime.datetime.now()
start_date = end_date - datetime.timedelta(days=3*365)

# fetch data for all tickers
data = yf.download(tickers, start=start_date, end=end_date, group_by='ticker')

# filter to only get close prices
close_data = data.xs('Close', level=1, axis=1)

# save to csv
close_data.to_csv('historical.csv')
