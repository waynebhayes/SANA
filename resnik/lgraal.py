#!/usr/bin/python
from fastsemsim.Ontology import ontologies
from fastsemsim.Ontology import AnnotationCorpus
import fastsemsim.SemSim as SemSim
import protores

DEBUG = False

def testRead(filename):
    """ Prints out first 5 values of LGRAAL output file.
    """
    yeast,human = readOutput(filename)
    for x in range(5):
        print yeast[x],human[x]
 
def readOutput(filename):
    yeast = []
    human = []
    with open(filename ,"r") as f:
        for line in f:
            try:
                yeastGene,humanGene = line.strip().split();
            except ValueError:
                continue
            else:
                yeast.append(yeastGene)
                human.append(humanGene)
    return yeast,human

def evalTerm(geneA, geneB, commonA, commonB, ac, evaluator,scorer):
    termsA = protores.getGOterms(ac, commonA, geneA)
    termsB = protores.getGOterms(ac, commonB, geneB)
    return evaluator.SemSim(termsA,termsB, scorer) 

if __name__ == "__main__":
    if DEBUG:
        testRead("output.txt")
    outputFile = "./data/mf.txt"

    ontology = ontologies.load("go.obo")
    acTransformA = "geneTemp.txt"
    acTransformB = "geneTemp2.txt"
    acSourceType = 'gaf-2.0'
    yeastAC = "gene_association.sgd"
    humanAC = "gene_association.goa_human"
    lgraalFile = "output.txt"

    ontologyFilter = {"F"}

    ac = AnnotationCorpus.AnnotationCorpus(ontology)
    protores.sortGAF(yeastAC, ontologyFilter, acTransformA)
    protores.sortGAF(humanAC, ontologyFilter, acTransformB)

    ac.parse(acTransformA, "gaf-2.0")

    nodeNumber = 16127

    ac.parse(acTransformB, "gaf-2.0")

    bma = SemSim.select_mix_SemSim('BMA')(ontology,ac)

    scoreFunction = SemSim.select_term_SemSim('Resnik')(ontology,ac)

    yeast,human = readOutput("output.txt")

    yeastCommon = protores.gafCommonTerm(acTransformA)
    humanCommon = protores.gafCommonTerm(acTransformB)
    scores = []
    for geneA, geneB in zip(yeastCommon,humanCommon):
        scores.append(evalTerm(geneA, geneB, yeastCommon, humanCommon, ac, bma, scoreFunction))

    avg = 0
    for x in scores:
        avg += x if x != None else 0

    avg = avg/nodeNumber

    scoresN = [x for x in scores if x != None]
    scoresN = [x for x in scoresN if x > 0]
    minimum = min(scoresN)
    maximum = max(scoresN)
    with open(outputFile,"w") as f:
        f.write("Average: {}\r\n".format(avg))
        f.write("Maximum: {}\r\n".format(maximum))
        f.write("Minimum: {}\r\n".format(minimum))



