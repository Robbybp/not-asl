import pyomo.environ as pyo
import argparse

def write_default_model():
    m = pyo.ConcreteModel()

    m.x = pyo.Var([1, 2, 3, 4, 5], initialize=1.0)
    m.eq = pyo.Constraint(pyo.Integers)
    m.eq[1] = m.x[1] + 2*m.x[2] + 4*m.x[3] == 3.5
    m.eq[2] = m.x[4] * m.x[1] == m.x[3]
    m.eq[3] = (m.x[1] + m.x[3]) / m.x[2]**2 == 1.1
    m.eq[4] = m.x[1] * m.x[2] * m.x[3] == 5.0
    m.eq[5] = m.x[5] - m.x[4] + m.x[3]**3 == m.x[1]

    m.obj = pyo.Objective(expr=sum(v**2 for v in m.x[:]))

    m.write("model.nl")


def write_model_with_unary():
    m = pyo.ConcreteModel()

    m.x = pyo.Var([1, 2, 3, 4, 5], initialize=1.0)
    m.eq = pyo.Constraint(pyo.Integers)
    # NOTE: unary sin operator here
    m.eq[1] = pyo.sin(m.x[1]) + 2*m.x[2] + 4*m.x[3] == 3.5
    m.eq[2] = m.x[4] - pyo.tan(m.x[4] * m.x[1]) == m.x[3]
    m.eq[3] = (m.x[1] + m.x[3]) / m.x[2]**2 == pyo.exp(m.x[1])

    # Note that this constraint contains a unary minus operator
    # This actually appears to be interpreted as a constant of -1, rather than
    # a unary minus, probably due to how Pyomo handles product expressions.
    m.eq[4] = m.x[1] * pyo.exp(m.x[2] * pyo.cos(m.x[3])) == 5.0
    m.eq[5] = m.x[5] - m.x[4] + pyo.sqrt(m.x[3]**3) == pyo.log(m.x[1])

    m.obj = pyo.Objective(expr=sum(v**2 for v in m.x[:]))

    m.write("model.nl")


if __name__ == "__main__":
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        "--model",
        default=None,
        help=(
            "Optional argument for writing models that test different"
            " functionality. Options = unary"
        ),
        )

    args = argparser.parse_args()

    if args.model is None:
        write_default_model()
    else:
        write_model_with_unary()
