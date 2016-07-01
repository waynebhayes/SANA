#!/usr/bin/env/ python
from fastsemsim.Ontology import ontologies
from fastsemsim.Ontology import AnnotationCorpus
import fastsemsim.SemSim as SemSim

from collections import defaultdict
from decimal import Decimal

from subprocess import call
import sys
import argparse

#ONTOLOGY_CONVERT = {"BP":"P", "MF":"F", "CC":"C"}
ONTOLOGY_CONVERT = {"BP":"Process", "MF":"Function", "CC":"Component"}

DEBUG = False

def geneList(fileName):
    with open(fileName,'r') as f:
        # Skips first 4 lines of a LEDA graph file
        for i in range(4):
            f.readline()
        count = int(f.readline().strip())
        return [f.readline().strip()[2:-2] for i in range(count)]

#def gafCommonTerm(fileName):
#    result = defaultdict(set)
#    gafFile = open(fileName,"r")
#    for line in gafFile:
#        if line[0] == '!':
#            continue
#        params = line.strip('\n').split('\t')
#        result[params[2]].add(params[1])
#        result[params[1]].add(params[1])
#        for geneAlternate in params[11].split('|'):
#            result[geneAlternate].add(params[1])
#    gafFile.close()
#    return result

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

#def getGOterms(ac, gafCommon, gene):
def getGOterms(ac, gene):
#    result = []
#    for geneName in gafCommon[gene]:
#        if geneName in ac.annotations:
#            result += list(ac.annotations[geneName].keys())
    try:
        return list(ac.annotations[gene].keys())
    except KeyError:
#    print(gene, "not in ac.annotations")
        return list()
def getAlignment(filename):
    with open(filename,'r') as f:
        return map(int, f.readline().strip().split())

def sortGAF(filename, aspects, output):
    sortString = "{{if ({}) print}}"
    arg = " || ".join('$8=="{}"'.format(aspect) for aspect in aspects)
    with open(output,'w') as f:
        call(["awk", "-F", "\t", sortString.format(arg), filename], stdout=f)

def evalTerm(geneA, geneB, ac, evaluator, scorer):
    termsA = getGOterms(ac, geneA)
    termsB = getGOterms(ac, geneB)
    return evaluator.SemSim(termsA, termsB, scorer) 

def initParser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-ont", "--ontology", required=True, type=str,
            help="Ontology file, obo format")
    parser.add_argument("-g1", "--graph1", required=True, type=str, 
            help="graph1, which graph2 is aligned to")
#    parser.add_argument("-ga1", "--graph-ac1", required=True, type=str, 
#            help="Annotation corpus of graph1")
    parser.add_argument("-g2", "--graph2", required=True, type=str,
            help="graph2")
#    parser.add_argument("-ga2", "--graph-ac2", required=True, type=str,
#            help="annotation corpus of graph2")
    parser.add_argument("-ac", "--annotation-corpus", required=True, type=str,
            help="Annotation Corpus, must be NCBI format")
    parser.add_argument("-t", "--taxon", type=str, 
            help="Taxonomies to include, separated by a '-'")
    parser.add_argument("-al", "--alignment-file", required=True, type=str,
            help="Alignment file")
    parser.add_argument("--mode", default="max", type=str,
            choices=["BMA","avg","max"], help="Evaluation mode")
    parser.add_argument("-ec", "--evidence-codes", type=str, help="Evidence Codes") 
    parser.add_argument("-i", "--include-ec", action="store_true", 
            help="Include evidence codes, default set to exclude")
    parser.add_argument("-oc", "--ontology-codes", type=str, help="Ontology Codes") 
    parser.add_argument("-o", "--output", default="output.txt", type=str, 
            help="Output file")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose")
    parser.add_argument("--mne", action="store_true", 
            help="Include MNE evaluation")
    parser.add_argument("--afs", action="store_true", 
            help="Include AFS evaluation")
    parser.add_argument("--semsim", default="Resnik", type=str,
            choices=[x for x in SemSim.term_SemSim_measures],
            help="Method of evaluating similarity")
    return parser

if __name__ == "__main__":

    acTransform = "geneTemp.txt"
#    acSourceType = "gaf-2.0"
    acSourceType = "ncbi"

    parser = initParser()
    args = parser.parse_args()
    evidenceCodes = set(args.evidence_codes.split("-")) if args.evidence_codes else {}
    ontologyCodes = set(args.ontology_codes.split("-")) if args.ontology_codes else set(ONTOLOGY_CONVERT.values()) 
    taxonCodes = set(args.taxon.split("-")) if args.taxon else {} 
    taxonInclude = True if args.taxon else False
    if args.verbose:
        verboseFormat = "{:20s} :  {}\r\n"
        print(verboseFormat.format("Graph 1", args.graph1))
        # print(verboseFormat.format("Annotation Corpus", args.graph_ac1))
        print(verboseFormat.format("Graph 2", args.graph2))
        print(verboseFormat.format("Annotation Corpus", args.annotation_corpus))
        print(verboseFormat.format("Alignment file", args.alignment_file))
        print(verboseFormat.format("Ontology file", args.ontology))
        print(verboseFormat.format("Include taxons", taxonCodes))
        choice = "{} evid. code".format(("Exclude","Include")[args.include_ec])
        print(verboseFormat.format(choice, evidenceCodes))
        print(verboseFormat.format("Include ontologies", ontologyCodes))
        print(verboseFormat.format("Output File", args.output))
        print(verboseFormat.format("Semantic Similarity", args.semsim))
        print(verboseFormat.format("Mode", args.mode))

    acFile = args.annotation_corpus

    if args.ontology_codes:
        sortGAF(acFile, ontologyCodes, acTransform)
        acFile = acTransform

    acParams = {}
    acParams["filter"] = {} # filter section is useful to remove undesired annotations
    acParams["filter"]["EC"] = {"EC" : evidenceCodes, "inclusive" : args.include_ec} 
    acParams["filter"]["taxonomy"] = {"taxonomy" : taxonCodes, "inclusive" : taxonInclude}

    geneListA = geneList(args.graph1)
    geneListB = geneList(args.graph2)

    alignment = getAlignment(args.alignment_file)
    compareList = [geneListB[i] for i in alignment]

    ontology = ontologies.load(args.ontology)

    ac = AnnotationCorpus.AnnotationCorpus(ontology)
    ac.parse(acFile, acSourceType, acParams)
    # commonA = gafCommonTerm(ac1)

    # ac.parse(ac2, acSourceType, acParams)
    # commonB = gafCommonTerm(ac2)

    ac.isConsistent()

    try:
        call(['rm',acTransform])
    except:
        pass

    evaluator = SemSim.select_mix_SemSim(args.mode)(ontology, ac)
    scoreFunction = SemSim.select_term_SemSim(args.semsim)(ontology, ac)

    scores = []

    for geneA,geneB in zip(geneListA, compareList):
        scores.append(evalTerm(geneA,geneB,ac,evaluator,scoreFunction))

    noScores = sum(1 for x in scores if not x)

    filterScores = [score for score in scores if score != None]

    scoreCount = len(scores)
    scoreMax = max(filterScores)
    scoreMin = min(filterScores)
    scoreAvg = sum(filterScores) / len(filterScores)
    # scoreAFS = sum(filterScores) / (scoreCount * (scoreCount- 1) / 2)
    # Need to implement AFS
    scoreAFS = 0
    print("Max  : {}\r\n".format(scoreMax))
    print("Min  : {}\r\n".format(scoreMin))
    print("Avg  : {}\r\n".format(scoreAvg))
    print("None : {}\r\n".format(noScores))
    if args.afs:
        print(("AFS  : {}\r\n".format(scoreAFS)))
    if args.mne:
        meanEntropy = MNE(geneListA, commonA, compareList, commonB, ac)
        print("MNE  : {}\r\n".format(meanEntropy))

    with open(args.output,'w') as f:
        for x,y,z in zip(geneListA, compareList, scores):
            f.write("{}\t{}\t{}\r\n".format(x,y,z))
