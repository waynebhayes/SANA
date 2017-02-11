#!/usr/bin/env/ python
import sys
#sys.path.append('/extra/wayne0/preserve/resnik/fastsemsim-code/fastsemsim/')
try:
    from fastsemsim.Ontology import ontologies
    from fastsemsim.Ontology import AnnotationCorpus
    from fastsemsim.SemSim.ObjSetSemSim import ObjSetSemSim
    import fastsemsim.SemSim as SemSim
except ImportError:
    print('Fastsemsim is not installed. Please install the fastsemsim package from SourceForge.')
    sys.exit(-1)

from collections import defaultdict
from decimal import Decimal

from subprocess import call
import sys
import os
import argparse

# List of constants we will be using
GENE_ONTOLOGY_FILE = "go.obo"
ONTOLOGY_CONVERT = {"BP" : "Process", "MF" : "Function", "CC" : "Component"}
ONTOLOGY_ROOT = {"BP" : 8150, "MF" : 3674, "CC" : 5575}
TAXON_CODES = {"Athaliana","CElegans","DMelanogaster","HSapiens", "MMusculus", "RNorvegicus",
               "SCerevisiae","SPombe"}
TAXON_FILES = {key : "taxons/" + key + ".txt" 
                for key in TAXON_CODES }

def geneList(fileName):
    with open(fileName,'r') as f:
        # Skips first 4 lines of a LEDA graph file
        for i in range(4):
            f.readline()
        count = int(f.readline().strip())
        return [f.readline().strip()[2:-2] for i in range(count)]

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

def getAlignment(filename):
    with open(filename,'r') as f:
        return map(int, f.readline().strip().split())

def sortGAF(filename, aspects, output):
    sortString = "{{if ({}) print}}"
    arg = " || ".join('$8=="{}"'.format(aspect) for aspect in aspects)
    with open(output,'w') as f:
        call(["awk", "-F", "\t", sortString.format(arg), filename], stdout=f)

def initParser():
    parser = argparse.ArgumentParser()
    parser.add_argument("-g1", "--graph1", required=True, type=str, 
            help="graph1, which graph2 is aligned to")
    parser.add_argument("-g2", "--graph2", required=True, type=str,
            help="graph2")
    parser.add_argument("-t", "--taxon", type=str, 
            help="Taxonomies to include, separated by a '-'")
    parser.add_argument("-so", "--sana-out", type=str,
            help="Alignment file")
    parser.add_argument("-a", "--alignment-file", type=str,
            help="Two column alignment file")
    parser.add_argument("--mode", default="max", type=str,
            choices=["BMA","avg","max"], help="Evaluation mode")
    parser.add_argument("-ec", "--evidence-codes", type=str, help="Evidence Codes") 
    parser.add_argument("-i", "--include-ec", action="store_true", 
            help="Include evidence codes, default set to exclude")
    parser.add_argument("-oc", "--ontology-root", default="BP-MF-CC", type=str, 
		    help="Ontology root, defaults to all. Separate by '-'") 
    parser.add_argument("-o", "--output", type=str, 
            help="Output file")
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose")
    parser.add_argument("--semsim", default="Resnik", type=str,
            choices=[x for x in SemSim.term_SemSim_measures],
            help="Method of evaluating similarity")
    return parser

def getTaxons(name):
    with open(TAXON_FILES[name], mode='r') as f:
        return [line.strip() for line in f]

if __name__ == "__main__":

    acSourceType = "ncbi"
    parser = initParser()
    args = parser.parse_args()
    ontologyRoots = {ONTOLOGY_ROOT[root] for root in args.ontology_root.split("-")}
    evidenceCodes = set(args.evidence_codes.split("-")) if args.evidence_codes else {}
    taxonCodes = []

    if args.taxon:
        for taxon in args.taxon.split("-"):
            taxonCodes.extend(getTaxons(taxon)) 
    else:
        taxon1 = os.path.basename(args.graph1)[:-3]
        taxon2 = os.path.basename(args.graph2)[:-3]
        taxonCodes.extend(getTaxons(taxon1) if taxon1 in TAXON_CODES else [])
        taxonCodes.extend(getTaxons(taxon2) if taxon2 in TAXON_CODES else [])

    taxonCodes = set(taxonCodes)
    taxonInclude = True if len(taxonCodes) else False

    # Display all parameters
    if args.verbose:
        verboseFormat = "{:20s} :  {}"
        print(verboseFormat.format("Graph 1", args.graph1))
        print(verboseFormat.format("Graph 2", args.graph2))
        print(verboseFormat.format("Annotation Corpus", "gene2go"))
        print(verboseFormat.format("Alignment file", args.alignment_file))
        print(verboseFormat.format("Ontology file", GENE_ONTOLOGY_FILE))
        print(verboseFormat.format("Include taxons", taxonCodes))
        choice = "{} evid. code".format(("Exclude","Include")[args.include_ec])
        print(verboseFormat.format(choice, evidenceCodes))
        print(verboseFormat.format("Include ontologies", ontologyCodes))
        print(verboseFormat.format("Output File", args.output))
        print(verboseFormat.format("Semantic Similarity", args.semsim))
        print(verboseFormat.format("Mode", args.mode))

    acFile = "gene2go"
    acParams = {}
    acParams["filter"] = {} # filter section to remove undesired annotations
    acParams["filter"]["EC"] = {"EC" : evidenceCodes, "inclusive" : args.include_ec}
    acParams["filter"]["taxonomy"] = {"taxonomy" : taxonCodes, "inclusive" : taxonInclude}

    if args.sana_out:
        geneListA = geneList(args.graph1)
        geneListB = geneList(args.graph2)

        alignment = getAlignment(args.sana_out)
        compareList = [geneListB[i] for i in alignment]
    
    if args.alignment_file:
        geneListA = []
        compareList = []
        with open(args.alignment_file, mode='r') as f:
            for line in f:
                geneA,geneB = line.strip().split()
                geneListA.append(geneA)
                compareList.append(geneB)

    if not (args.sana_out or args.alignment_file):
        print("No alignment file")
        sys.exit(-2);

    ontology = ontologies.load(GENE_ONTOLOGY_FILE)
    ac = AnnotationCorpus.AnnotationCorpus(ontology)
    ac.parse(acFile, acSourceType, acParams)
    ac.isConsistent()
    scoreFunction = ObjSetSemSim(ontology, ac, args.semsim, args.mode)

    if args.output:
        with open(args.output,'w') as f:
            for geneA,geneB in zip(geneListA, compareList):
                scores = [scoreFunction.SemSim(geneA, geneB, root) 
                        for root in ontologyRoots]
                try:
                    maxScore =  max(score for score in scores if score != None)
                except:
                    maxScore = None
                f.write("{}\t{}\t{}\t{}\r\n".format(args.mode,geneA,geneB,maxScore))
    else:
        for geneA,geneB in zip(geneListA, compareList):
            try:
                score = max(x for x in scores if x != None)
            except:
                score = None
            print(args.mode,geneA,geneB,score,sep="\t")
