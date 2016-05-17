#!/usr/bin/env/ python
from fastsemsim.Ontology import ontologies
from fastsemsim.Ontology import AnnotationCorpus
import fastsemsim.SemSim as SemSim

from collections import defaultdict
from decimal import Decimal

from subprocess import call
import sys
import argparse

ONTOLOGY_CONVERT = {"BP":"P", "MF":"F", "CC":"C"}
DEBUG = False

def geneList(fileName):
    with open(fileName,'r') as f:
        for i in range(4):
            f.readline()
        count = int(f.readline().strip())
        return [f.readline().strip()[2:-2] for i in range(count)]

def gafCommonTerm(fileName):
    result = defaultdict(set)
    gafFile = open(fileName,"r")
    for line in gafFile:
        if line[0] == '!':
            continue
        params = line.strip('\n').split('\t')
        result[params[2]].add(params[1])
        result[params[1]].add(params[1])
        for geneAlternate in params[11].split('|'):
            result[geneAlternate].add(params[1])
    gafFile.close()
    return result

def MNE(geneListA, commonA, geneListB, commonB, ac):
    count = dict()
    total = 0
    for geneA, geneB in zip(geneListA, geneListB):
        total += 2
        a = getGOterms(ac, commonA, geneA)
        b = getGOterms(ac, commonB, geneB)
        for termA in a:
            try:
                count[termA] += 1
            except KeyError:
                count[termA] = 1
        for termB in b:
            try:
                count[termB] += 1
            except:
                count[termB] = 1
    frac = lambda x : Decimal(count[x]) / Decimal(total)
    result = sum(map(lambda x : frac(x) * frac(x).log10() , count)) * Decimal(len(count)).log10()
    return result 

def getGOterms(ac, gafCommon, gene):
    result = []
    for geneName in gafCommon[gene]:
        if geneName in ac.annotations:
            result += list(ac.annotations[geneName].keys())
    return result

def getAlignment(filename):
    with open(filename,'r') as f:
        return map(int, f.readline().strip().split())

def sortGAF(filename, aspects, output):
    sortString = "{{if ({}) print}}"
    arg = " || ".join('$9=="{}"'.format(aspect) for aspect in aspects)
    with open(output,'w') as f:
        call(["awk", "-F", "\t", sortString.format(arg), filename], stdout=f)

def evalTerm(geneA, geneB, commonA, commonB, ac, evaluator, scorer):
    termsA = getGOterms(ac, commonA, geneA)
    termsB = getGOterms(ac, commonB, geneB)
    return evaluator.SemSim(termsA, termsB, scorer) 

if __name__ == "__main__":

    acTransform1 = "geneTemp.txt"
    acTransform2 = "geneTemp2.txt"
    acSourceType = "gaf-2.0"

    parser = argparse.ArgumentParser()
    parser.add_argument("-ont", "--ontology", required=True, type=str, help="Ontology file, obo format")
    parser.add_argument("-g1", "--graph1", required=True, type=str, 
            help="graph1, which graph2 is aligned to")
    parser.add_argument("-ga1", "--graph-ac1", required=True, type=str, 
            help="Annotation corpus of graph1")
    parser.add_argument("-g2", "--graph2", required=True, type=str,
            help="graph2")
    parser.add_argument("-ga2", "--graph-ac2", required=True, type=str,
            help="annotation corpus of graph2")
    parser.add_argument("-al", "--alignment-file", required=True, type=str,
            help="Alignment file")
    parser.add_argument("--mode", default="max", type=str,
            choices=["BMA","avg","max"], help="Evaluation mode")
    parser.add_argument("-ec", "--evidence-codes", type=str, help="Evidence Codes") 
    parser.add_argument("-i", "--include-ec", action="store_true", 
            help="Include evidence codes, default set to exclude")
    parser.add_argument("-oc", "--ontology-codes", type=str, help="Ontology Codes") 
    parser.add_argument("-o", "--output", default="output.txt", type=str, help="Output file")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose")
    parser.add_argument("--mne", action="store_true", help="Include MNE evaluation")
    parser.add_argument("--afs", action="store_true", 
            help="Include AFS evaluation")
    parser.add_argument("--semsim", default="Resnik", type=str,
            choices=[x for x in SemSim.term_SemSim_measures], help="Method of evaluating similarity")
    args = parser.parse_args()
    evidenceCodes = set(args.evidence_codes.split("-")) if args.evidence_codes else {}
    if args.verbose:
        verboseFormat = "{:20s} :  {}\r\n"
#        Print statements commented out until I get fastsemsim library to python 3
#        print verboseFormat.format("Graph 1", args.graph1)
#        print verboseFormat.format("Annotation Corpus", args.graph_ac1)
#        print verboseFormat.format("Graph 2", args.graph2)
#        print verboseFormat.format("Annotation Corpus", args.graph_ac2)
#        print verboseFormat.format("Alignment file", args.alignment_file)
#        print verboseFormat.format("Ontology file", args.ontology)
#        choice = "{} evid. code".format(("Exclude","Include")[args.include_ec])
#        print verboseFormat.format(choice, evidenceCodes)
#        print verboseFormat.format("Including ontologies", args.ontology_codes.split("-"))
#        print verboseFormat.format("Output File", args.output)
#        print verboseFormat.format("Semantic Similarity", args.semsim)
#        print verboseFormat.format("Mode", args.mode)

        sys.stdout.write(verboseFormat.format("Graph 1", args.graph1))
        sys.stdout.write(verboseFormat.format("Annotation Corpus", args.graph_ac1))
        sys.stdout.write(verboseFormat.format("Graph 2", args.graph2))
        sys.stdout.write(verboseFormat.format("Annotation Corpus", args.graph_ac2))
        sys.stdout.write(verboseFormat.format("Alignment file", args.alignment_file))
        sys.stdout.write(verboseFormat.format("Ontology file", args.ontology))
        choice = "{} evid. code".format(("Exclude","Include")[args.include_ec])
        sys.stdout.write(verboseFormat.format(choice, evidenceCodes))
        sys.stdout.write(verboseFormat.format("Including ontologies", args.ontology_codes.split("-")))
        sys.stdout.write(verboseFormat.format("Output File", args.output))
        sys.stdout.write(verboseFormat.format("Semantic Similarity", args.semsim))
        sys.stdout.write(verboseFormat.format("Mode", args.mode))

    ac1 = args.graph_ac1
    ac2 = args.graph_ac2

    if args.ontology_codes:
        ontologyCodes = set(ONTOLOGY_CONVERT[x] for x in args.ontology_codes.split('-'))
        sortGAF(ac1, ontologyCodes, acTransform1)
        sortGAF(ac2, ontologyCodes, acTransform2)
        ac1 = acTransform1
        ac2 = acTransform2

    acParams = {}
    acParams['filter'] = {} # filter section is useful to remove undesired annotations
    acParams['filter']['EC'] = {'EC' : evidenceCodes, 'inclusive' : args.include_ec} 

    geneListA = geneList(args.graph1)
    geneListB = geneList(args.graph2)

    alignment = getAlignment(args.alignment_file)
    compareList = [geneListB[i] for i in alignment]

    ontology = ontologies.load(args.ontology)

    ac = AnnotationCorpus.AnnotationCorpus(ontology)
    ac.parse(ac1, acSourceType, acParams)
    commonA = gafCommonTerm(ac1)

    ac.parse(ac2, acSourceType, acParams)
    commonB = gafCommonTerm(ac2)

    ac.isConsistent()

    try:
        call(['rm',acTransform1,acTransform2])
    except:
        pass

    evaluator = SemSim.select_mix_SemSim(args.mode)(ontology, ac)
    scoreFunction = SemSim.select_term_SemSim(args.semsim)(ontology, ac)

    scores = []

    for geneA,geneB in zip(geneListA, compareList):
        scores.append(evalTerm(geneA,geneB,commonA,commonB,ac,evaluator,scoreFunction))

    noScores = sum(1 for x in scores if not x)

    filterScores = [score for score in scores if score != None]

    scoreCount = len(scores)
    scoreMax = max(filterScores)
    scoreMin = min(filterScores)
    scoreAvg = sum(filterScores) / len(filterScores)
    scoreAFS = sum(filterScores) / (scoreCount * (scoreCount- 1) / 2)
#    print "Max  : {}\r\n".format(scoreMax)
#    print "Min  : {}\r\n".format(scoreMin)
#    print "Avg  : {}\r\n".format(scoreAvg)
#    print "None : {}\r\n".format(noScores)
    sys.stdout.write("Max  : {}\r\n".format(scoreMax))
    sys.stdout.write("Min  : {}\r\n".format(scoreMin))
    sys.stdout.write("Avg  : {}\r\n".format(scoreAvg))
    sys.stdout.write("None : {}\r\n".format(noScores))
    if args.afs:
#        print ("AFS  : {}\r\n".format(scoreAFS))
        sys.stdout.write("AFS  : {}\r\n".format(scoreAFS))
    if args.mne:
        meanEntropy = MNE(geneListA, commonA, compareList, commonB, ac)
#        print "MNE  : {}\r\n".format(meanEntropy)
        sys.stdout.write("MNE  : {}\r\n".format(meanEntropy))

    with open(args.output,'w') as f:
        for x in scores:
            f.write("{}\r\n".format(x))
