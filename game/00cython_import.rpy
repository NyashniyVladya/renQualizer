
python early in _vladya_cython_import:

    """
    Реализация импорта cython модулей, через обычный 'import'.
    """

    import sys
    from os import path

    def _add_to_path(directory):
        """
        Добавляет папку, заданную относительно директории папки game в os.path.
        Для корректного импорта сайтоновских модулей.
        """
        try:
            directory = renpy.loader.transfn(directory)
        except Exception:
            directory = None
        if not (directory and path.isdir(directory)):
            raise ValueError(__("Некорректная директория."))
        directory = path.abspath(directory)
        for _path in filter(path.isdir, map(path.abspath, sys.path[:])):
            if directory == _path:
                # Путь уже добавлен.
                return
        sys.path.insert(0, directory)

    _add_to_path("_vl_cython_libs")
