import bjoern

def app(env, start_response):
    print("Got some new requests: " + env['PATH_INFO'])
    start_response('200 ok', [])
    return [b"es funktioniert antwort aus python!!!"]


bjoern.run(app, 'localhost', 9000)