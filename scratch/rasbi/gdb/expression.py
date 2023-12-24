import gdb
import gdb.types
import gdb.printing


#class ExpressionPrinter(gdb.ValuePrinter):  # This can be used only on modern gdb
class ExpressionPrinter():
    """Printer for struct ExpressionT"""
    def __init__(self, value):
        self.value = value

    def to_string(self):
        expr_type = self._expr_to_enum(self.value["expr_type"])
        content = "Not implemented"
        if expr_type == "TYPE_CONS":
            content = self._cons_str()
        if expr_type == "SYMBOL":
            content = self._symbol_str()
        if expr_type == "TYPE_STRING":
            content = self._string_str()
        return expr_type + "\n" + content

    def display_hint(self):
        # TODO: Finish me
        return None

    def _expr_to_enum(self, numerical_value):
        enum_type = gdb.lookup_type("enum ExpressionType")
        if not enum_type:
            raise RuntimeError("Enum not found")
        result = "Unknown expression type"
        for f in enum_type.fields():
            if f.enumval & numerical_value: # This is a little hackish, but for now it works
                result = f.name # It is important to not return here yet
        return result

    def _cons_str(self):
        """Print TYPE_CONS value"""
        expr_typedef = gdb.lookup_type("struct ExpressionT").pointer()
        value = self.value["value_cons"]

        car = value["car"].cast(expr_typedef)
        if car:
            car = self._expr_to_enum(car["expr_type"])

        cdr = "null"
        if value["cdr"] != None:
            cdr = value["cdr"].cast(expr_typedef)
            cdr = self._expr_to_enum(cdr["expr_type"])

        return "car: " + str(car) + "\n" + "cdr: " + str(cdr)

    def _symbol_str(self):
        """Print SYMBOL value"""
        value = self.value["value_symbol"]
        strlen = int(value["size"])
        content = value["content"]
        result = "Value: '"
        for i in range(strlen):
            result += chr(content[i])
        result += "'"
        return result

    def _string_str(self):
        """Print TYPE_STRING"""
        value = self.value["value_memory"]
        strlen = int(value["taken"])
        ptr = value["mem"]
        result = "'"
        for i in range((strlen)):
            result += chr(ptr[i])
        result += "'"
        return result


class ExpressionPrinterHook(gdb.printing.PrettyPrinter):

    def __call__(self, val):
        if str(val.type) == "struct ExpressionT":
            return ExpressionPrinter(val)
        return None

# GDB calls us directly on source %
if __name__ == "__main__":
    gdb.printing.register_pretty_printer(
            gdb.current_objfile(),
            ExpressionPrinterHook("Expression Printer"), replace=True)
