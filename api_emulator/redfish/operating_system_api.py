# Operating Systems API File

"""
Only GET is supported for now 
Singleton API: GET, POST, PUT, PATCH, DELETE
"""

from flask_restful import Resource
import sys, traceback
import logging

from .templates.operatingsystem import format_operating_system_template

members = {}
INTERNAL_ERROR = 500

class OperatingSystemAPI(Resource):
    def __init__(self, **kwargs):
        pass


    # HTTP GET
    def get(self, ident):
        logging.info("Operating System API made a GET request.")
        try:
            resp = 404
            if ident in members:
                resp = members[ident]['OperatingSystem'], 200
        except Exception:
            traceback.print_exc()
            resp = INTERNAL_ERROR
        return resp
    

    # HTTP PUT
    def put(self, ident):
        return 'PUT is not supported by this resource'
    

    # HTTP POST
    def post(self, ident):
        return 'POST is not supported by this resource'
    

    #HTTP PATCH
    def patch(self, ident):
        return 'PATCH is not supported by this resource'


    def delete(self, ident):
        return 'DELETE is not supposed by this resource'


            
def CreateOperatingSystem(**kwargs):
    suffix_id = kwargs['suffix_id']
    if suffix_id not in members:
        members[suffix_id] = {}
    members[suffix_id]['OperatingSystem'] = format_operating_system_template(**kwargs)


