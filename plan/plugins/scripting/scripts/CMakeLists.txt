install(FILES scripts.rc DESTINATION ${DATA_INSTALL_DIR}/plan/scripts)

install(FILES
    resourcesimport.py
    busyinfoexport.py
    busyinfoimport.py
    busyinfoclear.py
    DESTINATION ${DATA_INSTALL_DIR}/plan/scripts/extensions)

install(FILES
    sample_project.py
    sample_busyinfocheck.py
    DESTINATION ${DATA_INSTALL_DIR}/plan/scripts/samples)
